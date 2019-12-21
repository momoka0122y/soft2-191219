#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv)
{

    if (argc != 3){
    fprintf(stderr,"usage: %s <txt filename> <binary filename>\n",argv[1]);
    return EXIT_FAILURE;
  }



  size_t size;






  FILE *fp;
  if ( (fp = fopen(argv[1],"r")) == NULL ){
    fprintf(stderr,"%s: cannot open file.\n",argv[1]);
    return EXIT_FAILURE;
  }

  fscanf(fp,"%zu",&size);
  double *d = (double*)malloc(sizeof(double)*size);
  double buf;
  for(int i = 0 ; i < size ; i++)
    fscanf(fp,"%lf",&buf);

  fclose(fp);










    if ( (fp = fopen(argv[2],"r")) == NULL ){
    fprintf(stderr,"%s: cannot open file.\n",argv[1]);
    return EXIT_FAILURE;  
  }
  // 最初にsizeof(size_t) 1個分をサイズ情報として出しておく
  fread(&size,sizeof(size_t),1,fp);
  // dポインタを先頭にからsize個のdoubleデータを出力

  fread(d,sizeof(double),size,fp);//dに入れてく　sizeの個数だけ読み込みをする
  fclose(fp);
  return EXIT_SUCCESS;

}