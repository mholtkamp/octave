# Wiiload

Wiiload is a tool for sending homebrew executables directly to a Wii running the Homebrew Channel over the network. This enables rapid iteration during development without needing to swap SD cards.

## Prerequisites

1. **devkitPro** installed with wiiload:
   - Windows: `wiiload.exe` at `C:\devkitPro\tools\bin\wiiload.exe`
   - Linux: `wiiload` available in PATH after devkitPro installation

2. **Homebrew Channel** installed on your Wii

3. **Network connection**: Both your PC and Wii must be on the same local network

## Finding Your Wii's IP Address

1. Launch the **Homebrew Channel** on your Wii
2. Press the **Home** button on your Wii Remote
3. The IP address is displayed at the top-left of the screen (e.g., `192.168.1.42`)

## Editor Configuration

1. Open **Preferences** > **External** > **Launchers**
2. In the **Wiiload (Wii Hardware)** section, enter your Wii's IP address
3. The setting is saved automatically

## Usage

### Build & Run On Device

1. Open the **Packaging** window (**File** > **Package Project**)
2. Select or create a build profile with **Platform** set to **Wii**
3. Click **Build & Run On Device**
4. A confirmation popup will appear
5. Ensure your Wii has the Homebrew Channel open and is on the network
6. Click **Send**

The build will be compiled and sent to your Wii. The Homebrew Channel will automatically launch the application.

### How It Works

Wiiload uses the `WIILOAD` environment variable to specify the target device:

```bash
# Windows (cmd)
set WIILOAD=tcp:192.168.1.42
wiiload MyGame.dol

# Linux/macOS
WIILOAD=tcp:192.168.1.42 wiiload MyGame.dol
```

The editor constructs and executes this command automatically based on your configured IP address.

## Troubleshooting

### "wiiload not available"

- Verify devkitPro is installed
- Windows: Check that `C:\devkitPro\tools\bin\wiiload.exe` exists
- Linux: Run `which wiiload` to verify it's in your PATH

### "Wii IP address not configured"

- Open **Preferences** > **External** > **Launchers**
- Enter your Wii's IP address in the **Wii IP Address** field

### Connection Failed

1. Verify your Wii is powered on with the Homebrew Channel open
2. Confirm both devices are on the same network
3. Check that no firewall is blocking the connection
4. Try pinging your Wii's IP address from your PC

### Slow Transfer

Wiiload transfers over TCP which can be slow on congested networks. Ensure your Wii has a stable network connection.

## Technical Details

| Item | Value |
|------|-------|
| Protocol | TCP |
| Default port | 4299 |
| Supported formats | `.dol`, `.elf` |
| Environment variable | `WIILOAD=tcp:<IP>` |

## See Also

- [Wii Platform Overview](Overview.md)
- [3DS 3dslink Documentation](../3DS/Overview.md)
- [Homebrew Channel](https://wiibrew.org/wiki/Homebrew_Channel) on WiiBrew
- [wiiload](https://wiibrew.org/wiki/Wiiload) on WiiBrew
