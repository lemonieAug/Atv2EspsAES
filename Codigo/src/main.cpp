#include <Arduino.h>
#include <esp_system.h>
#include <cstring>
#include <cstdio>

#if !defined(CONFIG_IDF_TARGET_ESP32)
#error Este teste requer ESP32 classico com UART2; confirmar chip antes do upload.
#endif

// UART0/USB: evidencias. UART2: apenas o protocolo de teste sem criptografia.
static HardwareSerial linkUart(2);
static constexpr uint32_t kPackets = 1000, kTimeoutUs = 250000;
static constexpr char kRole = LAB_ROLE_A ? 'A' : 'B';
static char rx[80], console[16];
static size_t rxUsed = 0, consoleUsed = 0;
static bool discardLine = false, running = false, waiting = false;
static uint32_t runId, sent, received, lost, errors, unexpected, sentAt;
static uint32_t nextSend, rxStarted, minRtt, maxRtt, runStarted;
static uint64_t sumRtt;

static void beginRun() {
  runId = esp_random();
  sent = received = lost = errors = unexpected = 0;
  minRtt = UINT32_MAX; maxRtt = 0; sumRtt = 0;
  waiting = false; running = true;
  runStarted = nextSend = millis();
  Serial.printf("RUN role=A id=%08lx packets=%lu timeout_us=%lu\n",
                (unsigned long)runId, (unsigned long)kPackets, (unsigned long)kTimeoutUs);
}

static void finishRun() {
  running = false;
  Serial.printf("SUMMARY role=A id=%08lx status=%s sent=%lu received=%lu lost=%lu "
                "errors=%lu unexpected=%lu rtt_min_us=%lu rtt_mean_us=%.2f "
                "rtt_max_us=%lu duration_ms=%lu\n",
                (unsigned long)runId,
                received == kPackets && !lost && !errors && !unexpected ? "PASS" : "FAIL",
                (unsigned long)sent, (unsigned long)received, (unsigned long)lost,
                (unsigned long)errors, (unsigned long)unexpected,
                (unsigned long)(received ? minRtt : 0),
                received ? double(sumRtt) / received : 0.0,
                (unsigned long)maxRtt, (unsigned long)(millis() - runStarted));
}

static void handleLine(const char *line) {
  if (!LAB_ROLE_A) {
    unsigned long id = 0, seq = 0;
    int end = 0;
    if (sscanf(line, "PING A %8lx %lu%n", &id, &seq, &end) == 2 &&
        end > 0 && line[end] == '\0' && seq >= 1 && seq <= kPackets) {
      char response[64];
      snprintf(response, sizeof(response), "PONG B %08lx %lu", id, seq);
      linkUart.println(response);
      Serial.printf("RX role=B text=\"%s\" TX text=\"%s\"\n", line, response);
    } else {
      ++errors;
      Serial.printf("RX_ERROR role=B malformed=%lu\n", (unsigned long)errors);
    }
    return;
  }
  char expected[64];
  snprintf(expected, sizeof(expected), "PONG B %08lx %lu",
           (unsigned long)runId, (unsigned long)sent);
  if (running && waiting && strcmp(line, expected) == 0) {
    // RTT medido no relogio de A, incluindo ida, processamento em B e volta.
    const uint32_t rtt = micros() - sentAt;
    ++received; sumRtt += rtt;
    if (rtt < minRtt) minRtt = rtt;
    if (rtt > maxRtt) maxRtt = rtt;
    waiting = false; nextSend = millis() + 5;
    Serial.printf("RX role=A text=\"%s\" rtt_us=%lu\n", line, (unsigned long)rtt);
  } else {
    ++unexpected;
    Serial.printf("RX_UNEXPECTED role=A text=\"%s\"\n", line);
  }
}

static void readLink() {
  if (rxUsed && uint32_t(micros() - rxStarted) > kTimeoutUs) {
    rxUsed = 0; discardLine = true; ++errors;
    Serial.printf("RX_ERROR role=%c incomplete_line\n", kRole);
  }
  while (linkUart.available()) {
    const char c = char(linkUart.read());
    if (c == '\r') continue;
    if (c == '\n') {
      if (!discardLine && rxUsed) { rx[rxUsed] = '\0'; handleLine(rx); }
      rxUsed = 0; discardLine = false;
    } else if (!discardLine) {
      if (rxUsed == 0) rxStarted = micros();
      if (rxUsed + 1 < sizeof(rx) && c >= 32 && c <= 126) rx[rxUsed++] = c;
      else {
        rxUsed = 0; discardLine = true; ++errors;
        Serial.printf("RX_ERROR role=%c invalid_or_oversize_line\n", kRole);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.printf("BOOT role=%c firmware=uart-only-v1 chip=%s revision=%d cores=%d "
                "flash_bytes=%lu psram_bytes=%lu efuse_mac=%012llx\n",
                kRole, ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(),
                (unsigned long)ESP.getFlashChipSize(), (unsigned long)ESP.getPsramSize(),
                (unsigned long long)ESP.getEfuseMac());
  // Este log posterior nao substitui a identificacao do chip antes da gravacao.
  linkUart.setRxBufferSize(512);
  linkUart.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.printf("UART_READY role=%c rx=%d tx=%d baud=115200 format=8N1 crypto=none\n",
                kRole, UART_RX_PIN, UART_TX_PIN);
  Serial.println("Comando USB RUN inicia uma sequencia em A quando ociosa.");
}

void loop() {
  readLink();
  while (Serial.available()) {
    char c = char(Serial.read());
    if (c == '\r') continue;
    if (c == '\n') {
      console[consoleUsed] = '\0';
      if (LAB_ROLE_A && !running && strcmp(console, "RUN") == 0) beginRun();
      consoleUsed = 0;
    } else if (consoleUsed + 1 < sizeof(console)) console[consoleUsed++] = c;
  }
  static bool autoStarted = false;
  if (LAB_ROLE_A && !autoStarted && millis() >= 15000) {
    autoStarted = true;
    if (!running) beginRun();
  }
  if (LAB_ROLE_A && running) {
    if (waiting && uint32_t(micros() - sentAt) >= kTimeoutUs) {
      ++lost; waiting = false; nextSend = millis() + 5;
      Serial.printf("TIMEOUT role=A id=%08lx seq=%lu\n",
                    (unsigned long)runId, (unsigned long)sent);
    }
    if (!waiting && int32_t(millis() - nextSend) >= 0) {
      if (sent == kPackets) finishRun();
      else {
        char request[64];
        ++sent;
        snprintf(request, sizeof(request), "PING A %08lx %lu",
                 (unsigned long)runId, (unsigned long)sent);
        sentAt = micros(); waiting = true;
        linkUart.println(request);
        Serial.printf("TX role=A text=\"%s\"\n", request);
      }
    }
  }
  delay(1);
}
