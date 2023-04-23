# Triangle Sample

## 引理&证明
>
定理1 对于在集合$S\subset R^{n}$上具有概率密度函数$f$的$n$维连续随机变量$X$，若在集合$T\subset R^{m}$上存在连续随机变量$Y$满足单调函数$Y=\phi(X)$，则其分布函数为$F_Y(y)=F_X(\phi^{-1}(y))$(单调递增)或$F_Y(y)=1-F_X(\phi^{-1}(y))$(单调递减)，其分布密度函数为$f_y(y)=f(\phi^{-1}(y))\mid \frac{d}{dy}\phi^{-1}(y)\mid$
>

证明

$$
F_Y(y) = P(Y\leq y) \\
       = P(\phi(x)\in (-\inf, y]) \\
       = P(X\in \phi^{-1}(-\inf, y])
$$

当$\phi(X)$单调递增时(比如把某个cdf当作随机变量):
$$
F_Y(y) = P(Y\leq y) \\
       = P(\phi(x)\in (-\inf, y]) \\
       = P(X\in \phi^{-1}(-\inf, y]) \\
       = P(X\leq\phi^{-1}(y)) \\
       = F_X(\phi^{-1}(y))
$$

当$\phi(X)$单调递减时:

$$
F_Y(y) = P(Y\leq y) \\
       = P(\phi(x)\in (-\inf, y]) \\
       = P(X\in \phi^{-1}(-\inf, y]) \\
       = P(X\geq\phi^{-1}(y)) \\
       = 1 - F_X(\phi^{-1}(y))
$$

当$\phi(X)$单调递增时，$\frac{d}{dy}\phi^{-1}(y)\geq0$:

$$
f_Y(y)=\frac{d}{dy}F_Y(y) \\
       = \frac{d}{dy}F_X(\phi^{-1}(y)) \\
       = \frac{d\phi^{-1}(y)}{dy}\Bigg(\frac{d}{d\phi^{-1}(y)}F_X(\phi^{-1}(y))\Bigg) \\
       = f_X(\phi^{-1}(y))\frac{d\phi^{-1}(y)}{dy} \\
       = f_X(\phi^{-1}(y))\mid\frac{d\phi^{-1}(y)}{dy}\mid \\
$$

当$\phi(X)$单调递减时，$\frac{d}{dy}\phi^{-1}(y)\leq0$:

$$
f_Y(y)=\frac{d}{dy}F_Y(y) \\
       = \frac{d}{dy}F_X(\phi^{-1}(y)) \\
       = \frac{d\phi^{-1}(y)}{dy}\Bigg(\frac{d}{d\phi^{-1}(y)}(1 - F_X(\phi^{-1}(y)))\Bigg) \\
       = -f_X(\phi^{-1}(y))\frac{d\phi^{-1}(y)}{dy} \\
       = f_X(\phi^{-1}(y))\mid\frac{d\phi^{-1}(y)}{dy}\mid \\
$$

综上, 当$\phi(X)$单调时, 均有:

$$
f_Y(y) = f_X(\phi^{-1}(y))\mid\frac{d\phi^{-1}(y)}{dy}\mid
$$

>
定理2 对于在集合$S\subset R^{n}$上服从均匀分布的$n$维连续随机变量$X$,若在集合$T\subset R^{n}$上存在连续随机变量$Y$与$X$满足线性变换$Y=AX+b$，矩阵$A$满秩，则$Y$也服从均匀分布.
>

证明:

显然线性变换$Y=AX+b$是单调且连续的, 根据定理1，有:
$$
f_Y(y) = f_X(\phi^{-1}(y))\mid\frac{d\phi^{-1}(y)}{dy}\mid \\
       = f_X(A^{-1}(y-b))\mid\frac{d}{dy}A^{-1}(y-b)\mid
       = f_X(A^{-1}(y-b))\frac{1}{\mid\det(A)\mid}
$$

由于$X$服从均匀分布，因此$f_X(A^{-1}(y-b))$为常数C，所以$f_Y(y)$也为常数.

