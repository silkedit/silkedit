# SilkEditパッケージAPIリファレンス

パッケージはNode.jsのAPIの他にrequire('silkedit')でSilkEdit独自のAPIを使用することができます。

このドキュメントはSilkEdit独自のAPIのリファレンスです。

SilkEditのAPIは[Qt Widgets](http://doc.qt.io/qt-5/qtwidgets-index.html)をベースにしており、各ウィジェットに対応するAPIをJavaScriptから呼ぶことができます。

現在ウィジェットの全てのAPIをサポートしているわけではなく、ウィジェットのプロパティ、シグナル、スロット、そしてSilkEditのAPIで定義されたメソッドにアクセスすることができます。

例えば、[Dialog]{@link module:silkedit.Dialog}は[QDialog](http://doc.qt.io/qt-5/qdialog.html)に対応したクラスです。[QDialog](http://doc.qt.io/qt-5/qdialog.html)クラスは[modal](http://doc.qt.io/qt-5/qdialog.html#modal-prop)というプロパティを持ちますが、これはJavaScriptのプロパティとして定義されており、以下のようにアクセスできます。

```
const silkedit = require('silkedit');
const dialog = new silkedit.Dialog();
console.log(dialog.modal);
```
[QDialog](http://doc.qt.io/qt-5/qdialog.html)は[exec](http://doc.qt.io/qt-5/qdialog.html#exec)というスロットがあり、これはJavaScriptのメソッドとして呼び出すことができます。

```
const silkedit = require('silkedit');
const dialog = new silkedit.Dialog();
// 空のダイアログが表示される。
dialog.exec();
```

[QDialog](http://doc.qt.io/qt-5/qdialog.html)は[finished](http://doc.qt.io/qt-5/qdialog.html#finished)というシグナルがあり、これはNode.jsの[EventEmitter](https://nodejs.org/api/events.html#events_class_eventemitter)として実装されています。

```
const silkedit = require('silkedit');
const dialog = new silkedit.Dialog();
// ダイアログが終了した時にemitされる。
dialog.on('finished', (result) => console.log('dialog finished. result=' + result));
dialog.exec();
```

# パッケージ開発ガイド

## ディレクトリ構成

パッケージは[Nodeモジュール](https://nodejs.org/api/modules.html)となっていますが、いくつか独自のルールがあります。

パッケージのディレクトリ構成は以下のようになっています。

```
my_package
├── config_definition.yml
├── index.js
├── keymap.yml
├── locales
│   ├── en
│   │   └── translation.yml
│   └── ja
│       └── translation.yml
├── package.json
├── menu.yml
└── toolbar.yml
```

## index.js

index.jsはパッケージのエントリポイントです。以下にindex.jsの例を示します。

```
const silkedit = require('silkedit');

module.exports = {
  activate: function() {
  },

  deactivate: function() {
  },

  commands: {
    "hello": () => {
      silkedit.alert(silkedit.tr("hello:hello", "Hello!"));
    }
  }
}
```

index.jsはNodeモジュールとなっており、activate, deactivate, commandsのプロパティを持つオブジェクトを公開しています。

activateはパッケージがロードされた時に呼び出されます。deactivateはパッケージが削除される時に呼び出されます。

commandsプロパティにはパッケージが提供するコマンドを登録します。上の例では"hello"がコマンド名となり、コマンドが実行された時には```silkedit.alert(silkedit.tr("hello:hello", "Hello!"));```が実行されます。

コマンド名はパッケージ名が自動的に修飾され、keymap.ymlやmenu.ymlでコマンド名を指定する時は<パッケージ名>.<コマンド名>のように指定する必要があります。上記のhelloコマンドは"hello.hello"というコマンドになります。

[silkedit.alert]{@link module:silkedit.alert}はアラートを表示する関数です。

[silkedit.tr]{@link module:silkedit.tr}はローカライズされた文字列を取得する関数です。locales/ja/translation.ymlに日本語ロケールの時に使用されるローカライズされた文字列がYAMLで定義されています。

## 設定

config_definition.ymlでパッケージ独自の設定をYAMLで定義することができます。ルートはconfigで、その中に設定を定義します。以下にvimパッケージの例を示します。

```
config:
  enable_on_startup:
    title: Enable Vim emulation on startup
    description: Enable Vim emulation on startup
    type: boolean
    default: false
```

enable_on_startupが設定となります。その中に連想配列でtitle, description, type, defaultを定義します。

|             |             |
|-------------|-------------|
| title       | 設定のタイトル |
| description | 説明         |
| type        | 設定の型      |
| default     | デフォルト値   |

パッケージの設定は設定ダイアログに表示されます。

titleは設定ダイアログで表示されます。

descriptionは設定ダイアログのツールチップとして表示されます。

typeは設定の型です。string, integer, number, booleanのいずれかを指定して下さい。

## キーマップ

keymap.ymlで定義します。

以下のようにkey, commandのセットを列挙します。

```
- { key: ctrl+b, command: move_cursor_left}
- { key: ctrl+f, command: move_cursor_right}
```

以下のようにargsでコマンドに引数を渡すことも可能です。

```{ key: ctrl+b, command: move_cursor, args: { operation: left }}```

キーマップには条件を付加することも可能です。例えばOS、選択しているプログラミング言語モードなどです。
条件の書式は2種類あります。

```
<key> <operator> <operand>の形式（1つ以上のスペース区切り）。
```

例えば```if: lang == cpp```を定義すると、言語モードがC++の時にのみ有効になるショートカットを設定できます。

```
- { key: ctrl+b, command: move_cursor_left, if: lang == cpp}
```

operatorは"==", "!="をサポートしています。

もう1つの書式は"<key>"のみ定義する場合です。この場合operatorは"==", operandは"true"に設定されます。

```
- { key: ctrl+cmd+i, command: vim.toggle_vim_emulation, if: on_mac }
- { key: ctrl+alt+i, command: vim.toggle_vim_emulation, if: on_windows }
```

上記のon_mac, on_windowsはSilkEdit組み込みの条件で、例えばon_macはMac上で動かす時にoperandがtrueになります。

パッケージ独自の条件を付加することも可能です。以下はvimパッケージの例です。

```
  const modeCond = {
    isSatisfied: (operator, operand) => {
      return isEnabled && silkedit.Condition.check(toModeText(mode), operator, operand);
    }
  }

  silkedit.Condition.add("vim.mode", modeCond);
```

{@link module:silkedit.Condition.add}で条件を追加できます。

vimパッケージをロードするとvim.modeという条件が使用できるようになります。

上記で定義したmodeCondのisSatisfied関数がtrueを返す時のみ以下のキーマップは有効になります。

```
- { key: i, command: vim.insert_mode, if: vim.mode == normal }
```

## メニュー

menu.ymlで定義します。以下に例を示します。

```
menu:
  - id: file
    menu:
    - label: 'Say Hello!'
      id: hello
      command: hello.hello
      if: on_mac
      before: save
```

|       |                                      |
|-------|                                      |
|label  |メニューに表示される文字列                  |
|id     |メニューのid。ローカライズやbeforeで指定されます|
|command|実行するコマンド                          |
|if     |表示する条件                             |
|before |指定したidのメニューの上に表示するようにします   |

上記のhelloメニューはMacでのみファイルメニューの中の保存メニューの上に表示されます。クリックすると"hello.hello"コマンドが実行されます。

## ツールバー

toolbars.ymlで定義します。以下に例を示します。

```
toolbars:
- label: File
  id: file
  items:
  - icon: resources/images/new.png
    command: new_file
    tooltip: "New File"
    id: new_file
  - icon: resources/images/open.png
    command: open
    tooltip: Open
    id: open
    if: on_mac
```

ツールバーはメニューと違い2階層しかありません。ルート要素でツールバーの定義、ツールバーの子要素でツールバーに表示するアイテムを定義します。

ツールバーは以下の項目で定義します。

|       |                                      |
|-------|                                      |
|label  |右クリックメニューに表示される文字列                 |
|id     |ツールバーのid。ローカライズやbeforeで指定されます|
|items  |実行するコマンド                          |
|if     |表示する条件                             |
|before |指定したidのツールバーの前に表示するようにします   |

ツールバーアイテムは以下の項目で定義します。

|       |                                             |
|-------|                                              |
|icon   |表示するアイコン                                  |
|tooltip|ツールチップに表示される文字列                       |
|id     |ツールバーアイテムのid。ローカライズやbeforeで指定されます|
|command|実行するコマンド                                  |
|if     |表示する条件                                     |
|before |指定したidのツールバーアイテムの前に表示するようにします   |

iconは絶対パス、もしくはtoolbars.ymlからの相対パスを指定します。

## ローカライズ

localesディレクトリに以下のようにロケールごとのディレクトリを作って翻訳ファイル(translation.yml)を置きます。

```
word_count
  locales
    ja
      translation.yml
    zh_CN
      translation.yml
    zh_TW
      translation.yml
```

ja/translation.ymlの例をいかに示します。

```
menu:
  word_count:
    label: 単語数カウント
    
word_count: 単語数
```

#### メニューの国際化

```menu.<id>.label``` のように定義します。

#### ツールバーの国際化

```toolbar.<id>.tooltip``` のように定義します。

#### JavaScriptの文字列の国際化

{@link module:silkedit.tr}を呼びます。

keyの形式は```<package>:<key>```です。

以下の呼び出しはword_countパッケージの対応するロケールのtranslation.ymlのword_countキーの文字列を使用します。見つからなければ"word count"の文字列が使用されます。

```
silkedit.tr("word_count:word_count", "word count")
```

## パッケージの公開

SilkEditのパッケージはパッケージマネージャから検索、インストール、削除が可能です。

パッケージを公開するにはSilkEditパッケージの[セントラルリポジトリ](https://github.com/silkedit/packages)にパッケージを登録する必要があります。

パッケージ自身のリポジトリもGithubに置く必要があります。

#### package.json

package.jsonはパッケージの名前やバージョンなどの情報を格納したファイルです。基本的な書き方は[npmのサイト](https://docs.npmjs.com/files/package.json)を参考にして下さい。

package.jsonのサンプル

```
{
  "name": "yaml-validator",
  "version": "0.1.0",
  "description": "YAML Validator",
  "main": "index.js",
  "repository": {
    "type": "git",
    "url": "https://github.com/silkedit/yaml-validator.git"
  },
  "author": {
    "name": "SilkEdit team"
  },
  "license": "MIT",
  "dependencies": {
    "js-yaml": "3.x"
  }
}
```

repositoryは現在Githubのみ対応しています。Githubのurlを記入して下さい。以下の様なGithubの短縮URLも使用可能です。

```"repository": "silkedit/hello"```

#### タグを付けてパッケージのリポジトリをGithubにプッシュする

package.jsonのversionと同じタグをパッケージのGithubリポジトリに登録する必要があります。

1. ```git tag 0.1.0``` でタグを付けます。

2. ```git push origin master --tags```でパッケージをpushする際にタグも一緒にpushします。

#### パッケージの新規登録

1. [https://github.com/silkedit/packages](https://github.com/silkedit/packages) をGithub上でフォークします。

1. 開発したパッケージのpackage.jsonの中身をpackages.jsonに追加して、上記リポジトリにプルリクエストを投げます。必ずversionに一致するタグがパッケージのリポジトリに存在するか確認して下さい。

1. プルリクエストがマージされるとパッケージマネージャから検索可能になります。

#### 更新

packages.jsonを更新してセントラルリポジトリにプルリクエストを投げます。versionを必ず更新して下さい。また必ず新versionに一致するタグがパッケージのリポジトリに存在するか確認して下さい。

#### 削除

packages.jsonからパッケージに該当する箇所を削除してセントラルリポジトリにプルリクエストを投げます。

#### helloサンプルパッケージ

[https://github.com/silkedit/hello](https://github.com/silkedit/hello)