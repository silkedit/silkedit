# SilkEditパッケージAPIリファレンス

パッケージはNode.jsのAPIの他にrequire('silkedit')でSilkEdit独自のAPIを使用することができます。

このドキュメントはSilkEdit独自のAPIのリファレンスです。

SilkEditのAPIは[Qt Widgets](http://doc.qt.io/qt-5/qtwidgets-index.html)をベースにしており、各ウィジェットに対応するAPIをJavaScriptから呼ぶことができます。

現在ウィジェットの全てのAPIをサポートしているわけではなく、ウィジェットのプロパティとスロット、そしてSilkEditのAPIで定義されたメソッドにアクセスすることができます。

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