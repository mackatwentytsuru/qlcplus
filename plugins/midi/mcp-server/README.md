# MCU Model Context Protocol (MCP) サーバー

このプロジェクトは、Mackie Control Universal (MCU) MIDIプロトコルをラップするMCPサーバーです。これにより、AIアシスタント（Claudeなど）が物理的なMCUデバイスを制御できるようになります。特に **Behringer X-Touch Extender** でテストされています。

## インストール

## インストール

PyPIにはまだ公開されていないため、GitHubから直接インストールするか、リポジトリをクローンしてローカルからインストールします：

```bash
# GitHubから直接インストールする場合（推奨）
pip install git+https://github.com/mackatwentytsuru/qlcplus.git#subdirectory=plugins/midi/mcp-server

# Claude Desktopなどで uvx を使ってGitHubから直接実行する場合
uvx --from git+https://github.com/mackatwentytsuru/qlcplus.git#subdirectory=plugins/midi/mcp-server mcu-mcp
```

## Claude Desktopでの実行方法

以下の設定を `claude_desktop_config.json` に追加してください：

```json
{
  "mcpServers": {
    "qlc-mcu": {
      "command": "uvx",
      "args": [
        "--from",
        "git+https://github.com/mackatwentytsuru/qlcplus.git#subdirectory=plugins/midi/mcp-server",
        "mcu-mcp"
      ]
    }
  }
}
```

## 主な機能

- モータライズドフェーダーの制御（14ビット精度）
- ボタンLEDの状態設定（オン/オフ/点滅）
- VPot LEDリングの制御
- LCDスクリブルストリップへのテキストの書き込み
- 実際のデバイス状態の読み取り、ハンドシェイク、および最近の入力の取得

正確なボタンマッピングと定数については、`mcu_protocol.py` を参照してください。
