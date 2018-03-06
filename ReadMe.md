# PBR

[Implementation Notes 1](https://dannyteng.bitcron.com/post/shi-shi-da-qi-xuan-ran)

[Implementation Notes 2](https://dannyteng.bitcron.com/post/li-jie-wei-biao-mian-li-lun)

![](https://github.com/wubugui/FXXKTracer/raw/master/pic/newblog/atm/dome1.png)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/newblog/atm/demo2.png)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/newblog/atm/demo3.png)

![](https://github.com/wubugui/FXXKTracer/raw/master/IL/3.jpg)

# GPU SVO

![](https://github.com/wubugui/FXXKTracer/raw/master/pic/QQ%E5%9B%BE%E7%89%8720161123162223.png)

![](https://github.com/wubugui/FXXKTracer/raw/master/pic/QQ%E5%9B%BE%E7%89%8720161123162254.png)

512X512X512体素化，128M体素链表，256MSVO结点，64M Brick Pool。


# Atmosphere

![](https://github.com/wubugui/FXXKTracer/raw/master/pic/%E6%9C%AA%E6%A0%87%E9%A2%98-1.png)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/17-11-39.jpg)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/17-16-41.jpg)
![](https://github.com/wubugui/FXXKTracer/raw/master/pic/17-17-11.jpg)


# Deferred Shading


![](https://github.com/wubugui/FXXKTracer/raw/master/pic/QQ%E5%9B%BE%E7%89%8720161123162035.jpg)

![](https://github.com/wubugui/FXXKTracer/raw/master/pic/17-38-38.jpg)






# RHI封装

大部RHI封装代码来自UE4，FastApp中的渲染器和Image来自[Framework3](http://www.humus.name/index.php?page=3D)

*注意：已经按照新的封装修改了shader部分代码，其他的未封装APP需要调整代码才能运行。*