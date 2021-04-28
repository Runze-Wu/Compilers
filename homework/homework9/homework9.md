## 编译原理——04-28作业

**学号:181860109   姓名:吴润泽**

### 6.6.1 (2)

| 产生式                           | 语义规则 |
| -------------------------------- | -------- |
| $S\to\textbf{for}(S_1;B;S_2)S_3$ | $$       |

$$
\begin{alignat}{2}
S_1.next=&newlabel()\\
B.true=&newlabel()\\
B.fals=&S.next\\
S_2.next=&S_1.next\\
S_3.next=&newlabel()\\
S.code=&S_1.code||label(S_1.next)\\
&||B.code||label(B.true)\\
&||S_3.code||label(S_3.next)\\
&||S_2.code||gen('goto',S_1.next)
\end{alignat}
$$

