Update
======

一个用于更新的解决方案(暂时only windows).

UpdateTool	项目是用于生成更新的工具. 比如有文件 main.exe, main.ini, main.dll, 可以生成main.exe.zip, main.ini.zip, main.dll.zip, filelist.xml, 然后可以将这些文件复制到一个http服务器.


libupdate	更新程序进行更新时, 将首先下载filelist.xml, 然后根据xml的内容进行更新. xml中的内容每项中的含意分别如下:

```
name 		:	源文件名.
compress 	:	压缩方式, 目前支持2种压缩方式, 分别为zip和gz.
filehash	:	压缩文件的md5.
md5			:	源文件的md5, 若更新位置的文件md5与xml中的md5不同, 则这个文件将被更新替换, 除有check特别标识的情况.
version		:	文件版本号.
size		:	源文件大小.
url			:	文件所在url位置, libupdate更新时, 就是从此链接下载对应的文件的.
check		:	表示只要检查到文件存在, 则不替换文件, 文件不存在时, 才会被下载更新.
command		:	用于对此文件的执行命令, 目前只支持regsvr命令. regsvr表示对此文件下载更新后, 调用命令: regsvr32.exe /s filename, 是用于注册COM.
```

libupdate是以库的形式实现, 使用这个库时, 只需要包含updater.hpp以及以及libupdate这个库就行了(比如单独编译时可只提供updater.hpp和libupdate.lib即可).



Update	是一个用libupdate实现的客户端更新测试, 用于参照实现.




使用方法大致流程:

```
	1. 准备条件, 需要一台http服务器, UpdateTool工具程序, 还有使用libupdate编写的更新客户端程序, 以及需要更新的程序.
		比如: http://127.0.0.1, 在这个Http服务器目录中创建一个专用于更新的目录update.
		比如用于更新的文件有: main.exe, main.ini, main.dll.

	2. 使用UpdateTool生成xml及压缩文件.

		打开UpdateTool, 在第一个编辑框(应用程序安装所在目录)双击, 选择"main.exe, main.ini, main.dll"文件所在的文件夹.
		若main.ini是需要被客户编辑的, 那么则需要在UpdateTool中'检查存在'这一栏双击选择'是', 这样更新程序更新时, 即使main.ini文件md5和http上的不一至, 也不会被覆盖.
		创建一个用于存放UpdateTool输出更新文件的文件夹, 比如c:\out_dir, 然后在UpdateTool第二个编辑框(更新文件输出路径)中双击, 并选择这个文件夹.
		在第三个编辑框(URL位置)中输出在http服务器的路径, 在这里如http://127.0.0.1/update.
		OK, 到此为止, 基本上确定了大致信息, 现在只需要点击"开始生成"按钮, UpdateTool将会压缩所有选择的文件和生成一个xml在指定的输出文件夹.

	3. 复制c:\out_dir这个文件夹中所有文件, 到http://127.0.0.1下update这个目录下, 更新服务的配置就完成了.

```


更新客户端的编写可以使用libupdate参照Update项目来实现.



