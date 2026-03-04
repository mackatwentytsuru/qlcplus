<p align="center">
  <a href="https://www.qlcplus.org/">
    <img src="resources/icons/png/qlcplus.png" alt="QLC+ Logo" height="60" />
  </a>
</p>

<h1 align="center">Q Light Controller+</h1>
<p align="center"><em>(Often abbreviated as "QLC+")</em></p>
<p align="center">
  <strong>Open-source lighting control for DMX, Art-Net, sACN and more.</strong><br/>
  Designed for live shows, theatre, architectural installations, and venues.
</p>

<p align="center">
  <a href="https://github.com/mcallegari/qlcplus/releases/latest">
    <img src="https://img.shields.io/github/v/release/mcallegari/qlcplus" alt="Latest release version badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/releases/latest">
    <img src="https://img.shields.io/github/release-date/mcallegari/qlcplus" alt="Release date badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/commits/master/">
    <img src="https://img.shields.io/github/commits-since/mcallegari/qlcplus/latest/master" alt="Commits since latest release badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/commits/master/">
    <img src="https://img.shields.io/github/commit-activity/w/mcallegari/qlcplus" alt="Weekly commit activity badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/actions">
    <img src="https://github.com/mcallegari/qlcplus/actions/workflows/build.yml/badge.svg" alt="Build status badge" /></a>
  <a href="https://coveralls.io/github/mcallegari/qlcplus?branch=master">
    <img src="https://coveralls.io/repos/github/mcallegari/qlcplus/badge.svg?branch=master" alt="Test coverage badge" /></a>
</p>

---

<p align="center">
  <a href="https://www.qlcplus.org/download">
    <img src="https://custom-icon-badges.demolab.com/badge/-Download_QLC+-blue?style=for-the-badge&logo=download&logoColor=white" alt="Download QLC+ badge" /></a>
  <a href="https://qlcplus.org/discover/raspberry-pi">
    <img src="https://custom-icon-badges.demolab.com/badge/-Raspberry_Pi-red?style=for-the-badge&logo=cpu&logoColor=white" alt="Raspberry Pi badge" /></a>
  <a href="https://merch.qlcplus.org">
    <img src="https://custom-icon-badges.demolab.com/badge/-Store-green?style=for-the-badge&logo=home&logoColor=white" alt="Official store badge" /></a>
</p>

## はじめに (カスタムフォーク)

> [!NOTE]
> これは独立してメンテナンスされている QLC+ の **カスタムフォーク** です。このような強力な基盤となるソフトウェアを作成した Massimo Callegari 氏およびオリジナル [Q Light Controller+](https://github.com/mcallegari/qlcplus) のすべての貢献者に深く感謝いたします。

**QLC+ (Custom Edition)** は、この素晴らしいオープンソースの照明制御ソフトウェアを基盤とし、最新のワークフローに合わせた最先端の特殊機能を追加したものです。

**このフォークの主な追加機能:**
1. **Behringer X-Touch MCU 統合**: 専用の入力プロファイル (`Behringer-X-Touch-Extender.qxi`) と MCU SysEx プロトコルの強化が含まれており、Behringer X-TOUCH シリーズをシームレスに使用できます。
2. **AI コントロール (MCP サーバー)**: コンパニオンとなる Model Context Protocol (MCP) Python サーバーが同梱されており、AIアシスタント (Claudeなど) が物理的なミキサーや仮想フェーダーを直接制御することができます。

### 🤖 AI 統合 (MCP サーバー)

AIツールと物理ハードウェアのギャップを埋めるため、このフォークにはMCPサーバー (`mcu-mcp-server`) が含まれています。
このサーバーは標準的なPythonパッケージとして配布されます。

**Claude DesktopでMCPサーバーを実行する方法:**
`uvx` (または `pip`) がインストールされていることを確認し、`claude_desktop_config.json` に以下を追加してください：

```json
"mcpServers": {
  "qlc-mcu": {
    "command": "uvx",
    "args": ["mcu-mcp-server"]
  }
}
```
*注意: Python MCP サーバーのソースコードは `plugins/midi/mcp-server` にあります。*

### Supported protocols

[![MIDI](https://img.shields.io/badge/MIDI-%23323330.svg?style=for-the-badge&logo=midi&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/midi)
[![OSC](https://img.shields.io/badge/OSC-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/osc)
[![HID](https://img.shields.io/badge/HID-%23323330.svg?style=for-the-badge&logo=applearcade&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/hid)
[![DMX](https://img.shields.io/badge/DMX-%23323330.svg?style=for-the-badge&logo=amazonec2&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/dmx-usb)
[![ArtNet](https://img.shields.io/badge/ArtNet-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/art-net)
[![E1.31/S.ACN](https://img.shields.io/badge/E1.31%20S.ACN-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/e1-31-sacn)
[![OS2L](https://img.shields.io/badge/OS2L-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/os2l)

### QLC+ ソーシャルメディア

[![Instagram](https://img.shields.io/badge/Instagram-%23E4405F.svg?style=flat-square&logo=Instagram)](https://www.instagram.com/qlcplus/) 
[![YouTube](https://img.shields.io/badge/YouTube-%23FF0000.svg?style=flat-square&logo=YouTube)](https://www.youtube.com/watch?v=I9bccwcYQpM&list=PLHT-wIriuitDiW4A9oKSDr__Z_jcmMVdi) 
[![Facebook](https://img.shields.io/badge/Facebook-%231877F2.svg?style=flat-square&logo=Facebook)](https://www.facebook.com/qlcplus)

## サポートとバグ報告

これは個人的なフォークです。MCPサーバーまたはX-Touchプロファイルに特有のバグに気付いた場合は、元のリポジトリ（アップストリーム） **ではなく** 、このリポジトリでイシューを開いてください。
QLC+の一般的な機能については、[公式ドキュメント](https://docs.qlcplus.org/) を参照してください。

### ビルド済みバイナリのダウンロード
自分でコンパイルする必要はありません！このGitHubリポジトリの **Releases** タブにアクセスして、これらのカスタム機能を標準で含んだ最新のWindowsインストーラーまたはZIPアーカイブをダウンロードしてください。


## QLC+ のビルド

コンパイルガイドとプラットフォーム固有の手順は、[GitHub Wiki](https://github.com/mcallegari/qlcplus/wiki) に記載されています。

#### 開発中の方へ

If you're regularly updating QLC+ sources with git pull, you may encounter compiler warnings, errors, or unresolved symbols. We strive to keep the `master` branch free of critical errors; however, dependencies between objects can sometimes cause issues, requiring a full package recompilation rather than just updating recent changes.

## 貢献について
### ソフトウェア開発

QLC+をさらに良くするためのコミュニティからの貢献を歓迎します。大きな変更に取り組む場合は、まず [Development Forum](https://www.qlcplus.org/forum/viewforum.php?f=12) でスレッドを立ち上げてください。詳細は [CONTRIBUTING.md](CONTRIBUTING.md) ドキュメントを必ずお読みください。

### 資金的な支援

これを読んでくださっているだけで、すでに感謝しています。照明を始めたばかりであれば、金銭的な支援の義務は全くありません。QLC+があなたに収益の機会をもたらした際には、サポートいただけると大変ありがたいです。GitHubスポンサーが推奨されるオプションです。

<img src="https://img.shields.io/github/sponsors/mcallegari" alt="GitHub Sponsors"> <a href="https://github.com/sponsors/mcallegari"><img src="https://img.shields.io/badge/sponsor-30363D?logo=GitHub-Sponsors&logoColor=#white" /></a>

もしご興味がありましたら、QLC+には[公式ストア](https://qlcplus-merch.myshopify.com)もあり、[衣類](https://qlcplus-merch.myshopify.com/collections/clothing)、[テーマ](https://qlcplus-merch.myshopify.com/collections/themes)、[Raspberry Piイメージ](https://qlcplus-merch.myshopify.com/products/qlc-raspberry-pi-image)、または専門家への[1対1のコンサルティング](https://qlcplus-merch.myshopify.com/collections/training-and-support)を購入することができます。


## ありがとうございます！

QLC+の成功は、惜しみなく時間とスキルを提供してくださった多くの個人の献身と専門知識のおかげです。以下のリストは、QLC+の構築において重要な役割を果たした注目すべき貢献者の方々を称えるものです。

![GitHub contributors](https://img.shields.io/github/contributors/mcallegari/qlcplus)

<details>
<summary>QLC+ 5</summary>
    
*   Eric Arnebäck (3D プレビュー機能)
*   Santiago Benejam Torres (カタロニア語 翻訳)
*   Luis García Tornel (スペイン語 翻訳)
*   Nils Van Zuijlen, Jérôme Lebleu (フランス語 翻訳)
*   Felix Edelmann, Florian Edelmann (フィクスチャ定義、ドイツ語 翻訳)
*   Jannis Achstetter (ドイツ語 翻訳)
*   Dai Suetake (日本語 翻訳)
*   Hannes Bossuyt (オランダ語 翻訳)
*   Aleksandr Gusarov (ロシア語 翻訳)
*   Vadim Syniuhin (ウクライナ語 翻訳)
*   Mateusz Kędzierski + smaks6 (ポーランド語 翻訳)

</details>

<details>
<summary>QLC+ 4</summary>

*   Jano Svitok (bugfix, new features and improvements)
*   David Garyga (bugfix, new features and improvements)
*   Lukas Jähn (bugfix, new features)
*   Robert Box (fixtures review)
*   Thomas Achtner (ENTTEC wing improvements)
*   Joep Admiraal (MIDI SysEx init messages, Dutch translation)
*   Florian Euchner (FX5 USB DMX support)
*   Stefan Riemens (new features)
*   Bartosz Grabias (new features)
*   Simon Newton, Peter Newman (OLA plugin)
*   Janosch Frank (webaccess improvements)
*   Karri Kaksonen (DMX USB Eurolite USB DMX512 Pro support)
*   Stefan Krupop (HID DMXControl Projects e.V. Nodle U1 support)
*   Nathan Durnan (RGB scripts, new features)
*   Giorgio Rebecchi (new features)
*   Florian Edelmann (code cleanup, German translation)
*   Heiko Fanieng, Jannis Achstetter (German translation)
*   NiKoyes, Jérôme Lebleu, Olivier Humbert, Nils Van Zuijlen (French translation)
*   Raymond Van Laake (Dutch translation)
*   Luis García Tornel (Spanish translation)
*   Jan Lachman (Czech translation)
*   Nuno Almeida, Carlos Eduardo Porto de Oliveira (Portuguese translation)
*   Santiago Benejam Torres (Catalan translation)
*   Koichiro Saito, Dai Suetake (Japanese translation)
</details>

<details>
<summary>Q Light Controller</summary>

*   Stefan Krumm (Bugfixes, new features)
*   Christian Suehs (Bugfixes, new features)
*   Christopher Staite (Bugfixes)
*   Klaus Weidenbach (Bugfixes, German translation)
*   Lutz Hillebrand (uDMX plugin)
*   Matthew Jaggard (Velleman plugin)
*   Ptit Vachon (French translation)
</details>

---

<p align="center">
<a href="https://github.com/mcallegari/qlcplus/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=mcallegari/qlcplus" />
</a>
</p>

---


## License
<a href="https://github.com/mcallegari/qlcplus/blob/master/COPYING">
  <img alt="GitHub License badge" src="https://img.shields.io/github/license/mcallegari/qlcplus?style=flat-square" />
</a>

Licensed under the **Apache 2.0** License.  See [COPYING](COPYING) for details.

---
<p align="center">Copyright © Heikki Junnila, Massimo Callegari</p>
<p align="center">
  <img src="https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++ badge" />
  <img src="https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white" alt="Qt badge" />
  <img src="https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake badge" />
  <img src="https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E" alt="JavaScript badge" />
</p>
