// このファイルはスタンダードmidiファイルをバイナリで吐き出す
// ヘッダチャンクには適当だと思われる値を入力してある

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
using namespace std;
#define HEAD 8
uint32_t Count = 0;
const int Maxprime = 719;
const int N = 10000;
const int NumOfKey = 128;

//ファイルの末尾にバイナリを入れる
void pop_back(ofstream &ofs, char one_bite){
    ofs.write((const char*)&one_bite, sizeof(one_bite));
    Count++;
}

void sound_start(ofstream &ofs, char key, char vel, char waiting_time = 0){
    pop_back(ofs,waiting_time);//待機時間
    pop_back(ofs,0x90);//スタート
    pop_back(ofs,key);//音程
    pop_back(ofs,vel);//ベロシティ
}
void sound_stop(ofstream &ofs, char key, char vel, char waiting_time=120){
    pop_back(ofs,waiting_time);//待機時間
    pop_back(ofs,0x80);//エンド
    pop_back(ofs,key);//音程
    pop_back(ofs,vel);//ベロシティ
}

int main(void)
{
    cout<<"Name file :";
    string filename;
    cin>>filename;
    filename.append(".mid");
    //out:書き込み専用　binary:バイナリモードで開く
    ofstream ofs(filename, ios_base::out | ios_base::binary);
    if (!ofs) {
        cerr << "ファイルオープンに失敗" << endl;
        exit(1);
    }
    
    //ヘッダチャンク
    u_int32_t head[HEAD] = {
        0x6468544D,
        0x06000000,
        0x01000100,
        0x544DE001,
        0x00006B72,
        0xFF005800,
        0x72500803,
        0x6E657365,
    };
    for (int i=0; i<HEAD ; i++) {
        ofs.write((const char*)&head[i], sizeof(head[i]));
    }
    
    Count=10;
    pop_back(ofs,0x63);
    pop_back(ofs,0x65);
    //****************作りたいノイズに関するアルゴリズムを記述（本体）************************
    //この部分は待機時間(0)に始まり待機時間(0)に終わる
    //
    //今回は時間軸を整数値に見立てiとし、鍵盤を一番低い音から順に素数に見立て、素因数分解をする
    //音量は、素因数の量によって変化させる
    //素数は2から数え、719(128個目の素数)まである。
    //ベロシティは１biteで表されるため、最小値を0x0F=15とする。そのため、15*17=255=0xFF より、17の同じ素因数を持つものまで対応できる。
    

    //始めは全て素数であると定義しておき,後ほど審査してふるい落とす（ふるい法）
    //1000の中に128個以上の素数があると記憶していたためざっくり今回はN=1000まで調べる
    bool is_prime_num[Maxprime+1];
    for (int i=0;i<Maxprime+1;i++){
        is_prime_num[i] = 1;
    }
    //2~Nまでその倍数をふるい落とす
    for (int i=2; i<Maxprime+1; i++) {
        //素数なら（始めは全て素数であると定義してある）d
        if (is_prime_num[i]==1) {
            //倍数をふるいにかける
            int j=2;
            while (i*j<=Maxprime+1 && i<Maxprime+1 && j<Maxprime+1) {
                is_prime_num[i*j]=0;
                j++;
            }
        }
        //素数じゃなければ
        else {//スルー
        }
    }
    //素数を持つ配列を作る
    int prime[NumOfKey];
    fill_n(prime, NumOfKey, 0);
    int countp=0;
    for (int i=2; i<Maxprime+1; i++) {
        if (countp>=NumOfKey) break; //鍵盤の数を素数の個数が超えてしまったら
        else if (is_prime_num[i]) {
            prime[countp]=i;
            countp++;
        }
    }
    cout<<prime[NumOfKey-1]<<endl;
    
    
    //全ての整数値について、登場する素因数の個数を格納する配列を作成
    int cntkey[N][NumOfKey];
    fill_n(cntkey[0], N*NumOfKey, 0);
    //和音を構成したい整数値の個数分ループ
    for(int i_time = 1; i_time<=N; i_time++){
        //素数128個分の出現個数を記録しておくための配列
        fill_n(cntkey[0],NumOfKey,0);
        //鍵盤の個数分ループ
        int removing_i = i_time;
        for(int key=0; key<NumOfKey; key++){
            //素数であれば
            while (removing_i != 0 && removing_i%prime[key]==0 && cntkey[i_time][key]< 17) {
                cntkey[i_time][key]++;
                removing_i/=prime[key];
                cout<<prime[key]<<" ";
            }
        }
        cout<<":"<<i_time<<endl;
    }
    for (int i_time=0; i_time<N; i_time++) {
        bool is_first_note=true;
        //和音開始
        for (int key=0; key<NumOfKey; key++) {
            if (cntkey[i_time][key]!=0) {
                sound_start(ofs,127-key,15*cntkey[i_time][key]);
            }
        }
        //和音それぞれを停止
        for (int key=0; key<NumOfKey; key++) {
            if (cntkey[i_time][key]!=0) {
                if(is_first_note){
                    sound_stop(ofs,127-key,15*cntkey[i_time][key]);
                    is_first_note=false;
                }
                else sound_stop(ofs,127-key,15*cntkey[i_time][key],0);
            }
        }
    }
    
    pop_back(ofs,0x00);//待機時間
    //*******************************************************************************
    
    //末尾チャンク
    pop_back(ofs,0xFF);
    pop_back(ofs,0x2F);
    pop_back(ofs,0x00);
    
    //バイナリサイズの自動挿入
    //シークバーの移動（beg=begin）
    ofs.seekp( 18, ios::beg );
    const uint32_t Sizedata=Count;
    if(Sizedata>=0xFFFFFFFF){
        cout<<"データセクションが長すぎます";
        ofs.close();
        exit(0);
    }
    else{
        pop_back(ofs,Sizedata>>(3*8));
        pop_back(ofs,(Sizedata>>(2*8))%(UCHAR_MAX+1));
        pop_back(ofs,(Sizedata>>8)%(UCHAR_MAX+1));
        pop_back(ofs,Sizedata%(UCHAR_MAX+1));
    }
    
    ofs.close();
    
    return 0;
    
}