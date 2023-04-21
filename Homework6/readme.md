# 作业6

在作业5中, 我们需要遍历所有的三角形mesh找到光线与其的交点, 如果模型中三角形数量较多, 那么速度则比较慢. 本次作业则使用BVH进行加速.

# Object
注意, 这里我们的Triangle继承自Object, 同时我们加载的一个模型也继承自Object. 只要它们提供对应的接口即可(这样即可保证bounding box之间的寓意连续).

# BVH

首先, BVH的构建先于渲染. 构建过程类似segment tree. query的时间复杂度为log(N).

## BVH构建

参考segment tree即可.

## SAH
为了解决原来左右划分后两个bounding box重叠的体积.


[a1, a2, ..., an-1, an]
任意i, 1-i找到的bounding box A和 i+1到n的bounding box B.
求A的area areaA和B的area areaB, 对数量进行加权. areaA * i / (areaA + areaB), areaB * (n - i) / (areaA + areaB)


## reference
+ [SAH](https://zhuanlan.zhihu.com/p/50720158)