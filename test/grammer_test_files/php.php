<?php
abstract class Alpha
{
	abstract public function methodAlpha($arg1, $arg2);

	protected function methodBeta($arg1)
	{
		echo "Alpha クラスで実装されたmathodBeta メソッド¥n";
	}
	
	public function __destruct()
	{
		echo "インスタンスを破棄、メモリを開放[Alpha::destruct]¥n";
	}
}

class Beta extends Alpha
{
	public function __construct()
	{
		$this->methodAlpha("","");
		$this->methodBeta("");
	}
	
	public function methodAlpha($arg1, $arg2)
	{
		echo "Beta クラスで実装されたmethodAlpha メソッド¥n";
	}
}

new Beta();
?>