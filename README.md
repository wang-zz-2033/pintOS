# pintOS

pintos Project 2 -- User Program

目前已完成任务2--Argument Passing 参数传递----------------------待修改process.c--process_execute()

记得把文档写了

实验要求官网：http://web.stanford.edu/~ouster/cgi-bin/cs140-spring20/pintos/pintos_3.html
可以用万能的chrome自动翻译来看

参考：https://wenku.baidu.com/view/eed6bbdaa48da0116c175f0e7cd184254b351ba8.html

----------------------------------------------------------------------------------------------------
当前任务：任务3--系统调用

参考：https://wenku.baidu.com/view/eed6bbdaa48da0116c175f0e7cd184254b351ba8.html
（书签：p7 p23）
https://zhuanlan.zhihu.com/p/104497182
https://github.com/Wang-GY/pintos-project2/blob/master/src/userprog/syscall.c


任务3目标：根据中断帧的指针，通过 esp 找到用户程序调用号和参数

　　根据系统调用号，转向系统调用功能函数，并将参数传入

　　执行系统调用具体功能（例如：创建文件）

　　执行完，将返回值写入 eax

待解决的问题：
userprog/syscall.c 第203行 struct thread在哪里定义过 （也就是需要理解为什么205行是cur->'RET'）  ——>在threads/thread.h
                   全部 修改下函数名，免得像完全复制的（狗头）
有人能告诉我file_node是在哪里定义的吗

************************个人记录*******************************************************
对于vscode提示process.c 514行，struct thread没有pagedir这个情况，建议修改threads/thread.h里定义的结构体thread
我发现不能抄百度的那个了，那个顶多算参考。因为你不知道file_node是什么。
开始尝试自己写。
wait to 修改userprog/process.c Line140 process_wait()

GITHUB上那个process.c，是一个自定义函数+程序运行函数，而文库的二者合一了。
syscall.c 30行 需要pfn[]吗？
96行 日后修改。
136、154行 vscode大姨妈了
                   
-----------------------------------------------------------------------------------------------------
关于在ubuntu上运行

组长的虚拟机炸了，暂时没法试验能不能用。
根据知乎的说法，直接复制src文件夹运行还全FAIL，是配置的问题。
所以要直接复制修改过的 文件 到文件夹再测试。
后面会给出修改过哪些文件（自己一个一个找哪些有中文注释也行，233）。

--------------------------------------------------------------------------------------------------
有看出来在syscall.c中大量运用了“用户栈+指针”的操作吗？
esp什么的
