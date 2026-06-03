#include <stdio.h>
int main()
{ printf("hello world\n");
    char msg[100];  // 存输入的文字
    printf("请输入内容：");
    scanf("%s", msg);
    printf("你输入的内容：%s", msg);
    return 0;
}
