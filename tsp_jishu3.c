#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h> // strtol のエラー判定用

// 町の構造体（今回は2次元座標）を定義
typedef struct
{
  int x;
  int y;
} City;

// 描画用
typedef struct
{
  int width;
  int height;
  char **dot;
} Map;

typedef struct answer
{
  double count_distance;
  int *route;
}Answer;

// 整数最大値をとる関数
int max(const int a, const int b)
{
  return (a > b) ? a : b;
}

// プロトタイプ宣言
// draw_line: 町の間を線で結ぶ
// draw_route: routeでの巡回順を元に移動経路を線で結ぶ
// plot_cities: 描画する
// distance: 2地点間の距離を計算
// solve(): TSPをといて距離を返す/ 引数route に巡回順を格納

void draw_line(Map map, City a, City b);
void draw_route(Map map, City *city, int n, const int *route);
void plot_cities(FILE* fp, Map map, City *city, int n, const int *route);
double distance(City a, City b);
Answer solve(const City *city, int n, int *route, int *visited);
Map init_map(const int width, const int height);
void free_map_dot(Map m);
City *load_cities(const char* filename,int *n,const int max_cities);
Answer search(int index, const City *city, int n, int *route,double demo_distance, int *visited, int *flags, double sum_v);
double sum_distance(int n, int *route,const City *city);



Map init_map(const int width, const int height)
{
  char **dot = (char**) malloc(width * sizeof(char*));
  char *tmp = (char*)malloc(width*height*sizeof(char));
  for (int i = 0 ; i < width ; i++)
    dot[i] = tmp + i * height;
  return (Map){.width = width, .height = height, .dot = dot};
}
void free_map_dot(Map m)
{
  free(m.dot[0]);
  free(m.dot);
}

City *load_cities(const char *filename, int *n,const int max_cities)
{
  City *city;
  FILE *fp;
  if ((fp=fopen(filename,"rb")) == NULL){
    fprintf(stderr, "%s: cannot open file.\n",filename);
    exit(1);
  }
  fread(n,sizeof(int),1,fp);
  assert( *n <= max_cities ); 
  city = (City*)malloc(sizeof(City) * *n);
  for (int i = 0 ; i < *n ; i++){
    fread(&city[i].x, sizeof(int), 1, fp);
    fread(&city[i].y, sizeof(int), 1, fp);
  }
  fclose(fp);
  return city;
}
int main(int argc, char**argv)
{
  // const による定数定義
  const int width = 70;
  const int height = 40;
  const int max_cities = 100;

  Map map = init_map(width, height);
  
  FILE *fp = stdout; // とりあえず描画先は標準出力としておく
  if (argc != 2){
    fprintf(stderr, "Usage: %s <city file>\n", argv[0]);
    exit(1);
  }
  int n;

  City *city = load_cities(argv[1],&n,max_cities);

  // 町の初期配置を表示
  plot_cities(fp, map, city, n, NULL);
  sleep(1);

  // 訪れる順序を記録する配列を設定
  int *route = (int*)calloc(n, sizeof(int));
  // 訪れた町を記録するフラグ
  int *visited = (int*)calloc(n, sizeof(int));

  Answer answer= solve(city,n,route,visited);//ここにsolveあるよおおおおおお
  double d=answer.count_distance;
  plot_cities(fp, map, city, n, answer.route);
  printf("total distance = %f\n", d);
  for (int i = 0 ; i < n ; i++){
    printf("%d -> ", answer.route[i]);
  }
  printf("0\n");

  // 動的確保した環境ではfreeをする
  free(route);
  free(visited);
  free(city);
  
  return 0;
}

// 繋がっている都市間に線を引く
void draw_line(Map map, City a, City b)
{
  const int n = max(abs(a.x - b.x), abs(a.y - b.y));
  for (int i = 1 ; i <= n ; i++){
    const int x = a.x + i * (b.x - a.x) / n;
    const int y = a.y + i * (b.y - a.y) / n;
    if (map.dot[x][y] == ' ') map.dot[x][y] = '*';
  }
}

void draw_route(Map map, City *city, int n, const int *route)
{
  if (route == NULL) return;

  for (int i = 0; i < n; i++) {
    const int c0 = route[i];
    const int c1 = route[(i+1)%n];// n は 0に戻る必要あり
    draw_line(map, city[c0], city[c1]);
  }
}

void plot_cities(FILE *fp, Map map, City *city, int n, const int *route)
{
  fprintf(fp, "----------\n");

  memset(map.dot[0], ' ', map.width * map.height); 

  // 町のみ番号付きでプロットする
  for (int i = 0; i < n; i++) {
    char buf[100];
    sprintf(buf, "C_%d", i);
    for (int j = 0; j < strlen(buf); j++) {
      const int x = city[i].x + j;
      const int y = city[i].y;
      map.dot[x][y] = buf[j];
    }
  }

  draw_route(map, city, n, route);

  for (int y = 0; y < map.height; y++) {
    for (int x = 0; x < map.width; x++) {
      const char c = map.dot[x][y];
      fputc(c, fp);
    }
    fputc('\n', fp);
  }
  fflush(fp);
}

double distance(City a, City b)
{
  const double dx = a.x - b.x;
  const double dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
}

Answer solve(const City *city, int n, int *route, int *visited)//nはcityの数
{
  // 以下はとりあえずダミー。ここに探索プログラムを実装する
  // 現状は町の番号順のルートを回っているだけ
  // 実際は再帰的に探索して、組み合わせが膨大になる。
  route[0] = 0; // 循環した結果を避けるため、常に0番目からスタート
  visited[0] = 1;
    for (int i = 1; i < n ; i++){
    visited[i] = 0; // 訪問済みでないことを0で初期化
  }
  int *demo_route = (int*)calloc(n, sizeof(int));
  demo_route[0] = 0; 
  for (int i = 0 ; i < n ; i++){
    demo_route[i] = i; // 訪問済みかチェック
  }
  double demo_distance=sum_distance(n,demo_route,city);
  free(demo_route);//枝切りの基準となるdemo_distanceを作る。これより長かったら切る。

  int *flags = (int*)calloc(n, sizeof(int));

  Answer shortest = search(1, city,  n, route,demo_distance, visited, flags, 0.0);
  
  free(flags);
  return shortest;

  
}

double sum_distance(int n, int *route,const City *city){
  // トータルの巡回距離を計算する
  // 実際には再帰の末尾で計算することになる
  double sum_d = 0;
  for (int i = 0 ; i < n ; i++){
     int c0 = route[i];
     int c1 = route[(i+1)%n]; // nは0に戻る
    sum_d += distance(city[c0],city[c1]);
  }
  return sum_d;
}

Answer  search(int index, const City *city, int n, int *route, double demo_distance ,int *visited, int *flags, double sum_d)
{ 
  int max_index = n;
  assert(index >= 0 && sum_d >= 0);
  // 必ず再帰の停止条件を明記する (最初が望ましい)
  if (index == max_index){
    const char *format_ok = ", total_value = %5.1f\n";
    for (int i = 0 ; i < max_index ; i++){
      printf("%d", route[i]);
    }

      printf(format_ok, sum_d);

      char *flags_char= (char*)malloc(sizeof(char)*100);
      for (int i = 0 ; i < max_index ; i++){
        flags_char[i]=route[i];
    }

      return (Answer){ .count_distance = sum_d + distance(city[route[max_index]],city[route[0]]), .route=route};
    
  }

  // 以下は再帰の更新式: 現在のindex の品物を使う or 使わないで分岐し、index をインクリメントして再帰的にsearch() を実行する
  
  Answer tmp= (Answer){ .count_distance = demo_distance*2};
  

  for (int i =1; i<n; i++){
    if (visited[i]==0){
      route[index] = i;
      visited[i]=1;
      Answer rrr=search(index+1, city,  n, route,demo_distance, visited, flags, sum_d + distance(city[route[index-1]],city[route[index]]));
      
      if(rrr.count_distance<tmp.count_distance){
        tmp=rrr;
      }
      visited[i]=0;
    }
    
  }
  return  tmp;
}   