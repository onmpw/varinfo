# varinfo 变量信息打印

varinfo 变量信息打印。 扩展比较简单，主要是打印变量的值的存储地址、类型、值、引用计数以及是否是引用变量。

#### 1. 安装
安装比较简单，就是按照PHP扩展的安装方式进行安装即可
```bash
# ce php_src/ext
# git clone https://github.com/onmpw/varinfo.git
# cd varinfo
# phpize 
# ./configure --with-php-config=php_install_dir/bin/php-config
# make && make install
```
然后在php.ini文件中引入 varinfo.so 扩展即可。

#### 2. 使用
该扩展仅仅只提供了一个方法
```
string varinfo(string $str_name)
```
###### 参数
$str_name 是参数的名称，字符串类型

###### 返回值
该函数返回打印的信息，以字符串的形式返回。

#### 3. 举例
* 字符串变量
```php
<?php
$a = "string";
$b = $a;

echo varinfo("a"),"\n";
echo varinfo("b"),"\n";
?>
```
执行结果
```
// php 7
a: (addr=> 0x110217080, type=> string, value=> string, refcount=> 1, is_ref=> 0 )
b: (addr=> 0x110217090, type=> string, value=> string, refcount=> 1, is_ref=> 0 )

// php5
a: (addr=> 0x10c3818b8, type=> string, value=> string, refcount=> 2, is_ref=> 0 )
b: (addr=> 0x10c3818b8, type=> string, value=> string, refcount=> 2, is_ref=> 0 )
```
* 数组变量
```php
<?php
$aArr = array("key"=>"value1","value2");

$bArr = $aArr;

echo varinfo("aArr"),"\n";
echo varinfo("bArr"),"\n";
?>
```
执行结果
```
// php 7
aArr: (addr=> 0x1102170c0, type=> array, key=> (addr=> 0x110261de0, type=> string, value=> value1, refcount=> 1, is_ref=> 0 ), 0=> (addr=> 0x110261e00, type=> string, value=> value2, refcount=> 1, is_ref=> 0 ), refcount=> 3, is_ref=> 0 )
bArr: (addr=> 0x1102170d0, type=> array, key=> (addr=> 0x110261de0, type=> string, value=> value1, refcount=> 1, is_ref=> 0 ), 0=> (addr=> 0x110261e00, type=> string, value=> value2, refcount=> 1, is_ref=> 0 ), refcount=> 3, is_ref=> 0 )

// php 5
aArr: (addr=> 0x10c3833c0, type=> array, key=> (addr=> 0x10c381a38, type=> string, value=> value1, refcount=> 1, is_ref=> 0 ), 0=> (addr=> 0x10c3831e8, type=> string, value=> value2, refcount=> 1, is_ref=> 0 ), refcount=> 2, is_ref=> 0 )
bArr: (addr=> 0x10c3833c0, type=> array, key=> (addr=> 0x10c381a38, type=> string, value=> value1, refcount=> 1, is_ref=> 0 ), 0=> (addr=> 0x10c3831e8, type=> string, value=> value2, refcount=> 1, is_ref=> 0 ), refcount=> 2, is_ref=> 0 )
```
* 对象变量
```php
<?php
class A 
{
	public $a;
	public $b=32;
	private $c=100;
	protected $d = 10;

	public function show()
	{
		echo $this->b,"\n";
		echo $this->c,"\n";
	}
}

$aobj = new A;
$bobj = $aobj;

echo varinfo("aobj"),"\n";
echo varinfo("bobj"),"\n";
?>
```
执行结果
```
// php 7
aobj: (addr=> 0x10e0170a0, type=> object, value=> class A { public $a=>(NULL, refcount=>0, is_ref=>0 ) public $b=>(addr=> 0x10e080038, type=> int, value=> 40, refcount=>0, is_ref=>0 )}, refcount=> 2, is_ref=> 0 )
bobj: (addr=> 0x10e0170b0, type=> object, value=> class A { public $a=>(NULL, refcount=>0, is_ref=>0 ) public $b=>(addr=> 0x10e080038, type=> int, value=> 40, refcount=>0, is_ref=>0 )}, refcount=> 2, is_ref=> 0 )

// php 5
aobj: (addr=> 0x10c380050, type=> object, value=> class A{ public $a=>(NULL, refcount=> 1, is_ref=> 0 ) public $b=>(addr=> 0x10c37ef30, type=> int, value=> 40, refcount=> 1, is_ref=> 0 )}, refcount=> 2, is_ref=> 0 )
bobj: (addr=> 0x10c380050, type=> object, value=> class A{ public $a=>(NULL, refcount=> 1, is_ref=> 0 ) public $b=>(addr=> 0x10c37ef30, type=> int, value=> 40, refcount=> 1, is_ref=> 0 )}, refcount=> 2, is_ref=> 0 ) 
```

#### 4. 总结
代码中，取值，地址等都是尽量是按照底层存储结构使用原生的代码写的。尽量避免了使用PHP提供的宏。有些地方为了兼容PHP7和PHP5不得已使用系统提供的宏或者方法。最初写这个扩展的目的就是为了了解php底层对于变量的存储结构和方式。
代码中还存在很多待优化和完善的地方。有时间慢慢进行优化。
