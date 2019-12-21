#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> // strtol, strtod, strerror
#include <errno.h> // strtol, strtod でerror を補足したい

// 以下は構造体の定義と関数のプロトタイプ宣言

// 構造体 Item
// 価値valueと重さweightが格納されている
//
typedef struct item
{
  double value;
  double weight;
}Item;

// 構造体 Itemset
// number分のItemを保持するための構造体
// Itemポインタをmallocする必要あり
typedef struct itemset
{
  int number;
  Item *item;
} Itemset;

typedef struct answer
{
  double count_value;
  char *flags;
}Answer;

// 関数のプロトサイプ宣言

// Itemset *init_itemset(int, int);
//
// itemsetを初期化し、そのポインタを返す関数
// 引数:
//  品物の個数: number (int)
//  乱数シード: seed (int) // 品物の値をランダムにする
// 返り値:
//  確保されたItemset へのポインタ
Itemset *init_itemset(int number, int seed);

void free_itemset(Itemset *list);

// Itemset *load_itemset(char *filename)
//
// ファイルからItemset を設定し、確保された領域へのポインタを返す関数 [未実装, 課題1]
// 引数:
//  Itemsetの必要パラメータが記述されたバイナリファイルのファイル名 filename (char*)
// 返り値:
//  Itemset へのポインタ
Itemset *load_itemset(char *filename);

// void print_itemset(const Itemset *list)
//
// Itemsetの内容を標準出力に表示する関数
void print_itemset(const Itemset *list);

// void save_itemset(char *filename)
//
// Itemsetのパラメータを記録したバイナリファイルを出力する関数 [未実装, テスト用]
// 引数:
// Itemsetの必要パラメータを吐き出すファイルの名前 filename (char*)
// 返り値:
//  なし
void save_itemset(char *filename);

// double solve()
//
// ソルバー関数: 指定された設定でナップサック問題をとく [現状、未完成]
// 引数:
//   品物のリスト: Itemset *list
//   ナップサックの容量: capacity (double)
// 返り値:
//   最適時の価値の総和を返す
//
Answer solve(const Itemset *list, double capacity);

// double search()
//
// 探索関数: 指定されたindex以降の組み合わせで、最適な価値の総和を返す
//  再帰的に実行する
// 引数:
//  指定index : index (int)
//  品物リスト: list (Itemset*)
//  ナップサックの容量: capacity (double)
//  実際にナップサックに入れた品物を記録するフラグ: flags (int*)
//  途中までの価値と重さ (ポインタではない点に注意): sum_v, sum_w
// 返り値:
//   最適時の価値の総和を返す
Answer search(int index, const Itemset *list, double capacity, int *flags, double sum_v, double sum_w);

// エラー判定付きの読み込み関数
int load_int(const char *argvalue);
double load_double(const char *argvalue);

int load_int(const char *argvalue)
{
  long nl;
  char *e;
  errno = 0; // errno.h で定義されているグローバル変数を一旦初期化
  nl = strtol(argvalue,&e,10);
  if (errno == ERANGE){
    fprintf(stderr,"%s: %s\n",argvalue,strerror(errno));
    exit(1);
  }
  if (*e != '\0'){
    fprintf(stderr,"%s: an irregular character '%c' is detected.\n",argvalue,*e);
    exit(1);
  }
  return (int)nl;
}
double load_double(const char *argvalue)
{
  double ret;
  char *e;
  errno = 0; // errno.h で定義されているグローバル変数を一旦初期化
  ret = strtod(argvalue,&e);
  if (errno == ERANGE){
    fprintf(stderr,"%s: %s\n",argvalue,strerror(errno));
    exit(1);
  }
  if (*e != '\0'){
    fprintf(stderr,"%s: an irregular character '%c' is detected.\n",argvalue,*e);
    exit(1);
  }
  return ret;
}


// main関数
// プログラム使用例: ./knapsack 10 20
//  10個の品物を設定し、キャパ20 でナップサック問題をとく
int main (int argc, char**argv)
{
  /* 引数処理: ユーザ入力が正しくない場合は使い方を標準エラーに表示して終了 */
  if (argc != 3&&argc != 4){
    fprintf(stderr, "usage: %s <the number of items (int)> <max capacity (double)>\n",argv[0]);
    exit(1);
  }
  
  // 個数の上限はあらかじめ定めておく
  const int max_items = 100;

  const int n = load_int(argv[1]);
  assert( n <= max_items ); // assert で止める

  const double W = load_double(argv[2]);
  assert( W >= 0.0);
  Itemset *items;
  if(argc == 4){
    
    FILE *fp;
    if ( (fp = fopen(argv[3],"rb")) != NULL ) {
        printf("open file %s\n",argv[3]);
        int number;
        fread(&number,sizeof(int),1,fp);
        if(n!=number){
          fprintf(stderr,"n is not right\n");
          return EXIT_FAILURE;
        }
        items = (Itemset*)malloc(sizeof(Itemset));
        Item *item = (Item*)malloc(sizeof(Item)*number);
        double *d = (double*)malloc(sizeof(double)*number);
        fread(d,sizeof(double),number,fp);
        for (int i = 0 ; i < number ; i++){
          item[i].value = d[i];
        } 
        fread(d,sizeof(double),number,fp);
        for (int i = 0 ; i < number ; i++){
          item[i].weight = d[i];
        } 
        *items = (Itemset){.number = number, .item = item};
    }
    else{
      fprintf(stderr,"cannot open file %s\n",argv[3]);
      return EXIT_FAILURE;
    }
    fclose(fp);
  }
  else{
    // 乱数シードを1にして、初期化 (ここは変更可能)
  int seed = 1; 
  items = init_itemset(n, seed);

  }
  printf("max capacity: W = %.f, # of items: %d\n",W, n);

  
  print_itemset(items);

  // ソルバーで解く
  Answer kotae = solve(items, W);


  // 表示する
  printf("----\nbest solution:\n");
  printf("value: %4.1f\n",kotae.count_value);
  printf("answer:");

  for (int i = 0 ; i < n ; i++){
      printf("%d", kotae.flags[i]);
    }
  free_itemset(items);
  printf("\n");
  return 0;
}


// 構造体をポインタで確保するお作法を確認してみよう
Itemset *init_itemset(int number, int seed)
{
  Itemset *list = (Itemset*)malloc(sizeof(Itemset));

  Item *item = (Item*)malloc(sizeof(Item)*number);

  srand(seed);
  for (int i = 0 ; i < number ; i++){
    item[i].value = 0.1 * (rand() % 200);
    item[i].weight = 0.1 * (rand() % 200 + 1);//
  }
  *list = (Itemset){.number = number, .item = item};
  return list;
}

// itemset の free関数
void free_itemset(Itemset *list)
{
  free(list->item);
  free(list);
}

// 表示関数
void print_itemset(const Itemset *list)
{
  int n = list->number;
  const char *format = "v[%d] = %4.1f, w[%d] = %4.1f\n";
  for(int i = 0 ; i < n ; i++){
    printf(format, i, list->item[i].value, i, list->item[i].weight);
  }
  printf("----\n");
}

// ソルバーは search を index = 0 で呼び出すだけ
Answer solve(const Itemset *list,  double capacity)
{
  // 品物を入れたかどうかを記録するフラグ配列 => !!最大の組み合わせが返ってくる訳ではない!!
  int *flags = (int*)calloc(list->number, sizeof(int));
  Answer max_value = search(0,list,capacity,flags, 0.0, 0.0);
  free(flags);
  return max_value;
}

// 再帰的な探索関数
Answer search(int index, const Itemset *list, double capacity, int *flags, double sum_v, double sum_w)
{
  int max_index = list->number;
  assert(index >= 0 && sum_v >= 0 && sum_w >= 0);
  // 必ず再帰の停止条件を明記する (最初が望ましい)
  if (index == max_index){
    const char *format_ok = ", total_value = %5.1f, total_weight = %5.1f\n";
    for (int i = 0 ; i < max_index ; i++){
      printf("%d", flags[i]);
    }
    if (sum_w < capacity){
      printf(format_ok, sum_v, sum_w);

      char *flags_char= (char*)malloc(sizeof(char)*100);
      for (int i = 0 ; i < max_index ; i++){
        flags_char[i]=flags[i];
    }

      return (Answer){ .count_value = sum_v, .flags=flags_char};
    } 
  }

  // 以下は再帰の更新式: 現在のindex の品物を使う or 使わないで分岐し、index をインクリメントして再帰的にsearch() を実行する
  
  flags[index] = 0;
  Answer v0= search(index+1, list, capacity, flags, sum_v, sum_w );
  

  flags[index] = 1;


  Answer v1= (Answer){ .count_value = 0};

  if (sum_w + list->item[index].weight<capacity){
    v1= search(index+1, list, capacity, flags , sum_v + list->item[index].value, sum_w + list->item[index].weight);
  }else{
    v1= (Answer){ .count_value = 0};
  }
  // 使った場合の結果と使わなかった場合の結果を比較して返す
if (v0.count_value < v1.count_value) {
  free(v0.flags);
}else{
  free(v1.flags);
}
  return  (v0.count_value > v1.count_value) ? v0 : v1;
}   