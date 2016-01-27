SilkEditのパッケージメニューの中のパッケージ開発、新しいパッケージをクリックします。

```hello``` という名前を入力してOKをクリックします。

helloパッケージのディレクトリが開かれるので、index.jsを開きます。

index.jsの中身は以下のようになっています。

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

[silkedit.alert]{@link module:silkedit.alert}はアラートを表示する関数です。

[silkedit.tr]{@link module:silkedit.tr}はローカライズされた文字列を取得する関数です。locales/ja/translation.ymlに日本語ロケールの時に使用されるローカライズされた文字列がYAMLで定義されています。

```
menu:
  hello:
    label: 挨拶

hello: こんにちは！
```

```silkedit.tr("hello:hello", "Hello!")```は、helloパッケージのlocales/ja/translation.ymlに定義されているhelloキーの文字列を探します。translation.ymlでは```hello: こんにちは！```が定義されているので、"こんにちは！"という文字列を取得できます。キーが見つからなかった場合"Hello!"という文字列が使用されます。


menu.ymlは以下のようになっています。

```
menu:
  - id: packages
    menu:
      - label: 'Say Hello!'
        id: hello
        command: hello.hello
```

```id: packages```はSilkEditのパッケージメニューを表します。menuを入れ子にすることで既存メニューの子メニューを作ることができます。

上記の例ではパッケージメニューの子メニューとしてhelloメニューを作成しています。labelは表示される文字列ですが、locales/ja/translation.ymlでhelloメニュー用の文字列が定義されているので、日本語ロケールでは"挨拶"というメニューになります。

commandでメニューをクリックした時に実行するコマンドを指定します。コマンドは```<パッケージ名>:<コマンド名>```のように指定します。```hello:hello```はhelloパッケージのhelloコマンドを指定しています。

SilkEditを再起動するとパッケージメニューの下に"挨拶"メニューが表示されているはずなので、クリックすると"こんにちは！"というアラートが表示されます。