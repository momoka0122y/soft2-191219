#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h> // strtol のエラー判定用
#include <time.h>

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
double solve(const City *city, int n, int *route);
void yama(const City *city, int n, int *route, int *nowroute,double *min);
Map init_map(const int width, const int height);
void free_map_dot(Map m);
City *load_cities(const char* filename,int *n);

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

City *load_cities(const char *filename, int *n)
{
  City *city;
  FILE *fp;
  if ((fp=fopen(filename,"rb")) == NULL){
    fprintf(stderr, "%s: cannot open file.\n",filename);
    exit(1);
  }
  fread(n,sizeof(int),1,fp);
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
  

  City *city = load_cities(argv[1],&n);
  assert( n > 1 && n <= max_cities); // さすがに都市数100は厳しいので
  // 町の初期配置を表示
  plot_cities(fp, map, city, n, NULL);

  // 訪れる順序を記録する配列を設定
  int *route = (int*)calloc(n, sizeof(int));

  const double d = solve(city,n,route);
  plot_cities(fp, map, city, n, route);
  printf("total distance = %f\n", d);
  for (int i = 0 ; i < n ; i++){
    printf("%d -> ", route[i]);
  }
  printf("0\n");

  // 動的確保した環境ではfreeをする
  free(route);
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

double solve(const City *city, int n, int *best_route)
{
  best_route[0] = 0; // 循環した結果を避けるため、常に0番目からスタート
  
  int nowroute[n];
  for (int i = 0 ; i < n ; i++){
    best_route[i] = i;
    nowroute[i]=best_route[i];
  }//数字を順番通りに回った時のroute

  double sum_d = 0;
  for (int i = 0 ; i < n ; i++){
    const int c0 = best_route[i];
    const int c1 = best_route[(i+1)%n]; // i=0の時はc1は0になる。
    sum_d += distance(city[c0],city[c1]);
  }//ここで数字の順番通りに回った場合の距離を出して、それをbest_distanceの初期値にしている。
  double best_distance=sum_d;
  srand((unsigned int)time(NULL));//乱数のseedを作成


  for(int k=0;k<10*n;k++){//山登りを（狭義）30*n回する
      for(int shufle=0;shufle<3*n;shufle++){
          int a=rand()%(n-1)+1;//1~(n-1)までの数
          int b=rand()%(n-1)+1;//1~(n-1)までの数
          int x=nowroute[a];
          nowroute[a]=nowroute[b];
          nowroute[b]=x;//0以外の二つの数を交換
      }//nowrouteをシャッフルする
      int good_route[n];
      for(int i=0;i<n;i++){
        good_route[i]=nowroute[i];
      }
      double sumd = 0;
      for (int i = 0 ; i < n ; i++){
          const int c0 = good_route[i];
          const int c1 = good_route[(i+1)%n]; //i=0の時はc1は0になる。
          sumd += distance(city[c0],city[c1]);
      }
      yama(city,n,good_route,nowroute,&sumd);
      if(sumd<best_distance){
          best_distance = sumd;
          for(int i=0;i<n;i++){
            best_route[i]=good_route[i];
          }
      }
  }
  return best_distance;
}

void yama(const City *city, int n, int *good_route, int *nowroute,double *min){
  short flag=0;//一回のステップの中で改善策が見つかったかどうか。
  for(int i=1;i<n-1;i++){
      for(int j=i+1;j<n;j++){//0以外の町から二つ町を交換させ、それで改善されたらgood_routeとする。
          double newdis=0;
          int tmp_route[n];
          for (int make_tmp_route=0;make_tmp_route<n;make_tmp_route++){
            tmp_route[make_tmp_route]=nowroute[make_tmp_route];
          }
          tmp_route[j]=nowroute[i];
          tmp_route[i]=nowroute[j];//nowrouteからi番目とj番目を交換したtmp_routeを作る。
          for (int k = 0 ; k < n ; k++){
            const int c0 = tmp_route[k];
            const int c1 = tmp_route[(k+1)%n]; 
            newdis += distance(city[c0],city[c1]);
          }//新しい距離を計算
          if(newdis<*min){
              flag=1;
              *min=newdis;
              for(int k=0;k<n;k++){
                  good_route[k]=tmp_route[k];
              }
          }//もし新しいルートが前回より短かったらgood_routeをそれに変える。　変更したってわかるようにflag=1にしとく
      }
  }
  
  if(flag!=0){
    for(int i=0;i<n;i++){
      nowroute[i]=good_route[i];
    }
    yama(city,n,good_route,nowroute,min);
  }//もし上のfor文の中で変更があった場合、こっからさらに最適経路を探せる場合があるので最適経路を探せる場合があるので
  else {
    printf("%f ",*min);
    for(int i=0;i<n;i++){nowroute[i]=good_route[i];
      printf("%d ",good_route[i]);
    }
    printf("\n");
    
  }
}
