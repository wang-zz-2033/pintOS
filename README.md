# pintOS

pintos Project 2 -- User Program

目前已完成任务2--Argument Passing 参数传递

记得把文档写了

实验要求官网：http://web.stanford.edu/~ouster/cgi-bin/cs140-spring20/pintos/pintos_3.html
可以用万能的chrome自动翻译来看

参考：https://wenku.baidu.com/view/eed6bbdaa48da0116c175f0e7cd184254b351ba8.html

----------------------------------------------------------------------------------------------------
当前任务：任务3--系统调用

参考：https://wenku.baidu.com/view/eed6bbdaa48da0116c175f0e7cd184254b351ba8.html
（只有这个能看了）
（书签：p7 p22）

待解决的问题：
userprog/syscall.c 第203行 struct thread在哪里定义过 （也就是需要理解为什么205行是cur->'RET'）
                   全部 修改下函数名，免得像完全复制的（狗头）

-----------------------------------------------------------------------------------------------------
关于在ubuntu上运行

组长的虚拟机炸了，暂时没法试验能不能用。
根据知乎的说法，直接复制src文件夹运行还全FAIL，是配置的问题。
所以要直接复制修改过的 文件 到文件夹再测试。
后面会给出修改过哪些文件（自己一个一个找哪些有中文注释也行，233）。
