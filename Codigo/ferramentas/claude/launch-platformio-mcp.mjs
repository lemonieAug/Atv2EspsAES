#!/usr/bin/env node

import { readdir, readFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import { spawn } from "node:child_process";

const pluginVersions = path.join(
  os.homedir(),
  ".codex",
  "plugins",
  "cache",
  "platformio-mcp",
  "platformio-mcp",
);

async function findInstalledPlugin() {
  let entries;
  try {
    entries = await readdir(pluginVersions, { withFileTypes: true });
  } catch {
    return undefined;
  }

  const versions = [];
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    const root = path.join(pluginVersions, entry.name);
    try {
      const manifest = JSON.parse(
        await readFile(path.join(root, ".codex-plugin", "plugin.json"), "utf8"),
      );
      if (manifest.name === "platformio-mcp" && manifest.version) {
        versions.push({ root, version: manifest.version });
      }
    } catch {
      // Ignore incomplete or unrelated plugin-cache entries.
    }
  }

  versions.sort((a, b) => b.version.localeCompare(a.version, undefined, { numeric: true }));
  return versions[0]?.root;
}

const installedRoot = await findInstalledPlugin();
const useNpm = !installedRoot;
const command = useNpm ? (process.platform === "win32" ? "npx.cmd" : "npx") : process.execPath;
const args = useNpm
  ? ["-y", "platformio-mcp@3.1.0", ...process.argv.slice(2)]
  : [path.join(installedRoot, "scripts", "launch-platformio-mcp.mjs"), ...process.argv.slice(2)];

const child = spawn(command, args, {
  cwd: process.env.CLAUDE_PROJECT_DIR || process.cwd(),
  env: { ...process.env, PIO_MCP_NO_BROWSER: "true" },
  stdio: "inherit",
  shell: false,
  windowsHide: true,
});

child.once("error", (error) => {
  console.error(`[platformio MCP launcher] ${error.message}`);
  process.exitCode = 1;
});

child.once("exit", (code, signal) => {
  if (signal) {
    process.kill(process.pid, signal);
    return;
  }
  process.exitCode = code ?? 1;
});

for (const signal of ["SIGINT", "SIGTERM"]) {
  process.once(signal, () => {
    if (!child.killed) child.kill(signal);
  });
}
