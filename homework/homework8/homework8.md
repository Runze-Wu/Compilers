## 编译原理——04-21作业

**学号:181860109   姓名:吴润泽**

### 6.4.3

#### 中间代码

$$
\begin{aligned}
&t_1=i\times16\\
&t_2=j\times4\\
&t_3=t_1+t_2\\
&t_4=b[t_3]\\
&t_5=t_4\times12\\
&t_6=k\times4\\
&t_7=c[t_6]\\
&t_8=t_7\times4\\
&t_9=t_5+t_8\\
&t_{10}=a[t_9]\\
&x=t_{10}
\end{aligned}
$$ {three-term}

#### SDT

<img src="F:\Compilers\Compilers\homework\homework8\homework8.assets\sdttree.png" alt="sdttree" style="zoom:80%;" />

### 6.4.8

1. $A[3,4,5]=[(3-1)\times5\times6+(4-0)\times6+(5-5)]\times8=672$
2. $A[1,2,7]=[(1-1)\times5\times6+(2-0)\times6+(7-5)]\times8=112$
3. $A[4,3,9]=[(4-1)\times5\times6+(3-0)\times6+(9-5)]\times8=896$