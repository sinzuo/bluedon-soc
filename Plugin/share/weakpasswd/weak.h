#ifndef __WEAK_H__
#define __WEAK_H__

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * Function:weak_getscanprogress()
 * Description:取出最新的扫描进度
 * Input:....
 * Return: 进度0-100
 * Remark: 请在执行扫描后在读取结果信息
*/
int weak_getscanprogress(void *workspace);

/*
 * Function:weak_getreulst()
 * Description:取出最新的扫描结果
 * Input:....
 * Return: 成功 数据指针,失败 NULL
 * Remark: 请在执行扫描后在读取结果信息
*/
char* weak_getreulst(void *workspace);
/*
 * Function:weak_stopscan()
 * Description:停止本次扫描任务
 * Input:....
 * Return: ..
 * Remark: ..
*/
void weak_stopscand(void *workspace);

/*
 * Function:weak_main()
 * Description:弱密码扫描主函数
 * Input: __ipStr 一个合法的IP地址"172.16.3.33"或范围""172.16.3.2-30"
 * Return:0 成功,其他出错异常
 * Remark:
*/
char* weak_main(void *workspace);

void *weak_createworkspace(char *ipStr);
void weak_freeworkspace(void *workspace);

#ifdef __cplusplus
}
#endif

#endif
