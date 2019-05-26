#include <iostream>
#include <fstream>	//Arquivos
#include <windows.h>//FOREGROUND_INTENSITY
#include <string>	//Usar String
#include <dirent.h> //Classe para manipular diretórios
using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
//Constantes
#define mx			119		//Colunas da Matriz
#define my			30		//Linhas da Matriz
#define UP			72		//Tecla Cima
#define DOWN		80		//Tecla Baixo
#define RIGHT		77		//Tecla Direita
#define LEFT		75		//Tecla Esquerda
#define EXEC_TIME	 0		//Tempo que demora pra execucao (aumente caso os barris ou o player estejam muito rapidos)
#define BAR_TIME	15		//Tempo de Criacao dos Barris
#define BAR_QTD		40		//Quantidade de Barris

//Classe pra cor nos textos
class text_color {
public:
    enum {
    	BLACK			=0,
		BLUE			=1,
		GREEN			=2,
		CYAN			=3,
		RED				=4,
		MAGENTA			=5,
		BROWN			=6,
		LIGHTGRAY		=7,
 		DARKGRAY		=8,
		LIGHTBLUE		=9,
 		LIGHTGREEN		=10,
 		LIGHTCYAN		=11,
 		LIGHTRED		=12,
 		LIGHTMAGENTA	=13,
 		YELLOW			=14,
 		WHITE			=15,
        BRIGHT 			=FOREGROUND_INTENSITY,
        BGRAY			=BACKGROUND_INTENSITY,
    };
public:
    text_color() { hout = GetStdHandle(STD_OUTPUT_HANDLE); }
    void set(int color) { SetConsoleTextAttribute(hout, color); }
    void reset() { SetConsoleTextAttribute(hout, WHITE); }
private:
    HANDLE hout;
};
text_color color;

struct Player{ int x,y,sx,sy,vidas,pulo,mapa; }jogador;
struct Barrel{ int x,y; }barril[BAR_QTD];
struct DK    { int x,y; }dk;
struct Prince{ int x,y; }princesa;
struct Cursor{ int x,y; }cursor;
struct Mapa  {char map[my][mx]; string name,path; int RecordAtual,Record,tempo; } mapa;

string mapascustom[99];

//Variaveis
int  jumptime=EXEC_TIME,AppRunning=1,TempoIni,CreatorRunning=0,customopcao;//Tempos de pulo e coisas do menu (0= desligado, 1= "pressione...", 2= "Start Game", 4= "Create Map")
char esquerda=174,direita=175,chao=205,parede=186;	//Caracteres que nao sao reconhecidos pelos numeros (  « ,  » , - ,  ¦)
char troca=95,barrel=184,barra=124,princes=244,chaopri=220;//Mais caracteres ( _ ,  © , | )
bool GameRunning=false,KeyRunning=false,GameOver=false,GameWon=false;//Variavel do jogo rodando

void CarregaMapa(string NomeMapa, string PathMapa)
{
	char record[50];
	//Diretorio do arquivo
		char*path = new char[PathMapa.length() + 1];
		char*nome = new char[NomeMapa.length() + 1];

		strcpy(path, PathMapa.c_str());
		strcpy(nome, NomeMapa.c_str());

		strcat(path,nome);
	//
	ifstream arquivo;
    arquivo.open(path);
    while (!arquivo.eof()){
        for (int i=0; i<my; i++){
            for (int j=0; j<mx; j++){
				arquivo>>mapa.map[i][j];
            	if(mapa.map[i][j]==80){	//Verificar no arquivo onde esta o player(P)
            		mapa.map[i][j]=95;	//Transforma o "P" em "_" (underline)
            		jogador.x=j;	//Da as cordenadas do P para a variavel do jogador
            		jogador.y=i;
            		jogador.sx=j;	//Da as cordenadas de onde o jogador comeca
            		jogador.sy=i;
            	}
				else if(mapa.map[i][j]==princes){ princesa.x=j; princesa.y=i; }//Mesmo que o de cima, porem com a Princesa (¶)
				else if(mapa.map[i][j]==33){
						dk.x=j;		//Mesmo que o de cima, porem com o DK (!)
						dk.y=i;
					for(int a=0; a<BAR_QTD; a++){
						barril[a].x=j;	//Coloca os barris no mesmo ponto que o DK(coisa da minha programacao <3)
						barril[a].y=i;
					}
				}
			}
		}
		arquivo.getline(record,50);
		arquivo.getline(record,50);
		mapa.RecordAtual=atoi(record);
	}
    arquivo.close();
}
void SalvaMapa(string NomeMapa, string PathMapa)//Nao precisa de comentarios ne? <3
{
	//Diretorio do arquivo
		string ExteMapa=".txt";

		char*path = new char[PathMapa.length() + 1];
		char*nome = new char[NomeMapa.length() + 1];
		char*exte = new char[ExteMapa.length() + 1];

		strcpy(path, PathMapa.c_str());
		strcpy(nome, NomeMapa.c_str());
		strcpy(exte, ExteMapa.c_str());

		strcat(path,nome);
		strcat(path,exte);
	//
	mapa.map[jogador.sy][jogador.sx]=80;//Devolve o local inicial do jogador a matriz
	
	ofstream arquivo;
    arquivo.open(path);
    for (int i=0; i<my; i++){
        for (int j=0; j<mx; j++){arquivo<<mapa.map[i][j];}
        arquivo<<"\n";
    }
    arquivo<<mapa.Record;
    arquivo.close();
}

void clearScreen()	//Funcao do Heleno pra "limpar" a tela
{					//Na verdade apenas coloca o ponteiro na posicao 0,0 e coloca pra substituir
    HANDLE hOut;
    COORD Position;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}
void LimpaTela()	//Limpar a tela para nao ficar aqueles escritos atras
{
	clearScreen();
	for(int c=0; c<3720; c++){cout<<" ";}//ALGUEM POR FAVOR MELHORA ISSO!!!!!!(Mentira, o Pedrao-Senpai elogio isso porque da um fadeout *-* )
}
void ExibeMapa()
{
	bool GambiarraDosBarril=false;								//Gambiarra dos Barris, funciona igual ao descer escada :P
    clearScreen();
    for (int i=0; i<my; i++){
        for (int j=0; j<mx; j++){ 								//Verifica cada bloquinho da matriz, substituindo por:

        	GambiarraDosBarril=false;							//Barril com Gambiarra :'(
        	if(mapa.map[i][j]!=33)
        		for(int b=0; b<BAR_QTD; b++)
        			if((i==barril[b].y)&&(j==barril[b].x))		{color.set(text_color::YELLOW | text_color::BRIGHT); cout<<barrel;
																	GambiarraDosBarril=true; break;}

        	if(GambiarraDosBarril!=true){
            	if ((i==jogador.y)&&(j==jogador.x))				{color.set(text_color::LIGHTRED); cout<<"P";}							//Jogador
            	else if(mapa.map[i][j]==princes)				{color.set(text_color::LIGHTMAGENTA); cout<<mapa.map[i][j];}			//Princesa
            	else if(mapa.map[i][j]==chaopri)				{color.set(text_color::YELLOW      ); cout<<mapa.map[i][j];}			//ChaoPrincesa
            	else if(mapa.map[i][j]==esquerda)				{color.set(text_color::LIGHTMAGENTA); cout<<troca;}						//esquerda
            	else if(mapa.map[i][j]==direita)				{color.set(text_color::LIGHTMAGENTA); cout<<troca;}						//direita
            	else if(mapa.map[i][j]==95)						{color.set(text_color::LIGHTMAGENTA); cout<<troca;}						//Player ComeÃƒ §a
            	else if(mapa.map[i][j]==45)						{color.set(text_color::CYAN); cout<<mapa.map[i][j];}					//Meio Escada
            	else if(mapa.map[i][j]==barra)					{color.set(text_color::CYAN); cout<<mapa.map[i][j];}					//Lateral Escada
				else if(mapa.map[i][j]==33)						{color.set(text_color::BLUE |text_color::BRIGHT); cout<<mapa.map[i][j];}//DK
            	else 											{color.reset(); cout<<mapa.map[i][j];}									//Resto do mapa
            }
        }
        cout<<endl;
    }
    color.set(text_color::LIGHTRED);
    cout<<"\x03";
    color.reset();
    cout<<" x "<<jogador.vidas<<"   || x: "<<jogador.x<<" | y: "<<jogador.y<<" || Map: "<<mapa.name
		<<" || Time: "<<mapa.tempo<<" || Recorde: "<<mapa.RecordAtual<<"      ";
}
void ExibeCriador()
{
	if(CreatorRunning!=1){
		clearScreen();
		cout<<endl<<endl<<endl<<endl;
		cout<<"\t\t\t _                     _"	<<endl
			<<"\t\t\t(  _    _   |\\/| _  _ ' )"<<endl
			<<"\t\t\t_)(_||/(/_  |  |(_||_) !"	<<endl
			<<"\t\t\t                   |";
		color.reset();
		cout<<endl<<endl;
		cout<<endl<<"\t\t\t";
		if(CreatorRunning==2){ color.set(text_color::LIGHTRED); cout<<direita<<" Yes!  "; color.reset(); }else{ cout<<"  Yes!  "; }
		cout<<endl<<"\t\t\t";
		if(CreatorRunning==3){ color.set(text_color::LIGHTRED); cout<<direita<<" No!   "; color.reset(); }else{ cout<<"  No!   "; }
		cout<<endl<<"\t\t\t";
		if(CreatorRunning==4){ color.set(text_color::LIGHTRED); cout<<direita<<" Cancel"; color.reset(); }else{ cout<<"  Cancel"; }
	}
	if(CreatorRunning==1){
		color.reset();
	    clearScreen();
	    for (int i=0; i<my; i++){
	        for (int j=0; j<mx; j++){ 							//Verifica cada bloquinho da matriz, substituindo por:

	        	if((i==cursor.y)&&(j==cursor.x))	{//Se o cursor estiver no digito
	            	 if(mapa.map[i][j]==80)			{color.set(text_color::LIGHTRED    |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Jogador
	        	else if(mapa.map[i][j]==princes )	{color.set(text_color::LIGHTMAGENTA|BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Princesa
	        	else if(mapa.map[i][j]==chaopri )	{color.set(text_color::YELLOW      |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//ChaoPrincesa
	        	else if(mapa.map[i][j]==esquerda)	{color.set(text_color::LIGHTMAGENTA|BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//esquerda
	            else if(mapa.map[i][j]==direita)	{color.set(text_color::LIGHTMAGENTA|BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//direita
	            else if(mapa.map[i][j]==95)			{color.set(text_color::LIGHTMAGENTA|BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Player ComeÃƒ §a
	            else if(mapa.map[i][j]==45)			{color.set(text_color::CYAN        |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Meio Escada
	            else if(mapa.map[i][j]==barra)		{color.set(text_color::CYAN        |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Lateral Escada
				else if(mapa.map[i][j]==33)			{color.set(text_color::BLUE        |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//DK
	            else 								{color.set(text_color::WHITE       |BACKGROUND_INTENSITY); cout<<mapa.map[i][j];}//Resto do mapa
				}									 //Senão XD
	            else if(mapa.map[i][j]==80)			{color.set(text_color::LIGHTRED    ); cout<<mapa.map[i][j];}//Jogador
	        	else if(mapa.map[i][j]==princes )	{color.set(text_color::LIGHTMAGENTA); cout<<mapa.map[i][j];}//Princesa
	        	else if(mapa.map[i][j]==chaopri )	{color.set(text_color::YELLOW      ); cout<<mapa.map[i][j];}//ChaoPrincesa
	            else if(mapa.map[i][j]==esquerda)	{color.set(text_color::LIGHTMAGENTA); cout<<mapa.map[i][j];}//esquerda
	            else if(mapa.map[i][j]==direita)	{color.set(text_color::LIGHTMAGENTA); cout<<mapa.map[i][j];}//direita
	            else if(mapa.map[i][j]==95)			{color.set(text_color::LIGHTMAGENTA); cout<<mapa.map[i][j];}//Player Comeca
	            else if(mapa.map[i][j]==45)			{color.set(text_color::CYAN        ); cout<<mapa.map[i][j];}//Meio Escada
	            else if(mapa.map[i][j]==barra)		{color.set(text_color::CYAN        ); cout<<mapa.map[i][j];}//Lateral Escada
				else if(mapa.map[i][j]==33)			{color.set(text_color::BLUE        ); cout<<mapa.map[i][j];}//DK
	            else 								{color.reset(); cout<<mapa.map[i][j];}						//Resto do mapa
	        }
	        cout<<endl;
	    }
	    color.reset();
	    cout<<"Cursor - x: "<<cursor.x<<" y: "<<cursor.y<<" || Map Name: "<<mapa.path<<mapa.name<<"         ";
	}
}
void ExibeMenu()
{
	clearScreen();
	// Coloquei pq e mais facil pra editar, mas depois vou criar um txt e passar pra la
	if ((GetTickCount() % 800) < 400)
		color.set(text_color::RED | text_color::BRIGHT);
	else
		color.set(text_color::YELLOW | text_color::BRIGHT);
	cout<<endl<<endl;
	cout<<" oooooooooo.                         oooo                                 oooo    oooo"									<<endl
		<<" `888'   `Y8b                        `888                                 `888   .8P'"									<<endl
		<<"  888      888  .ooooo.  ooo. .oo.    888  oooo   .ooooo.  oooo    ooo     888  d8'     .ooooo.  ooo. .oo.    .oooooooo"	<<endl
		<<"  888      888 d88' `88b `888P'Y88b   888 .8P'   d88' `88b  `88.  .8'      88888[      d88' `88b `888P'Y88b  888' `88b"	<<endl
		<<"  888      888 888   888  888   888   888888.    888ooo888   `88..8'       888`88b.    888   888  888   888  888   888"	<<endl
		<<"  888     d88' 888   888  888   888   888 `88b.  888    .o    `888'        888  `88b.  888   888  888   888  `88bod8P'"	<<endl
		<<" o888bood8P'   `Y8bod8P' o888o o888o o888o o888o `Y8bod8P'     .8'        o888o  o888o `Y8bod8P' o888o o888o `8oooooo."	<<endl
		<<"                                                          .o..P'                                            d'     YD"	<<endl
		<<"                                                          `Y8P'                                             'Y88888P'"	<<endl
																																	<<endl
		<<"                                                      |\\/| _  _| _   |_     |~) _ |_  _  _ ~|~ _  _  _ _"				<<endl
		<<"                                                      |  |(_|(_|(/_  |_)\\/  |_)(_|| |(/_(_| | (/_(_|| | |"				<<endl
		<<"                                                                        /"												;
	if(AppRunning==1){cout<<endl<<endl<<endl<<endl<<endl<<endl<<"\t\t\t\t\t\tPress Enter to continue !!!";}
	else {
		color.reset();
		cout<<endl<<endl;
		cout<<endl<<"\t\t\t";
		if(AppRunning==2){ color.set(text_color::LIGHTRED); cout<<direita<<" Start Game       "; color.reset(); }else{ cout<<"  Start Game       "; }
		cout<<endl<<"\t\t\t";
		if(AppRunning==3){ color.set(text_color::LIGHTRED); cout<<direita<<" Play a Custom Map"; color.reset(); }else{ cout<<"  Play a Custom Map"; }
		cout<<endl<<"\t\t\t";
		if(AppRunning==4){ color.set(text_color::LIGHTRED); cout<<direita<<" Create Map       "; color.reset(); }else{ cout<<"  Create Map       "; }
		cout<<endl<<"\t\t\t";
		if(AppRunning==5){ color.set(text_color::LIGHTRED); cout<<direita<<" Exit             "; color.reset(); }else{ cout<<"  Exit             "; }
	}
	color.reset();
}
void ExibeTeclado()				//Exibe o teclado antes de entrar no modo
{
	clearScreen();
	cout<<endl<<endl;
	if(AppRunning==4){//Caso voce entre no modo criador
	cout<<"\t                                                                                     Valid Keys"		<<endl
		<<"\t                                                                       .--------------."				<<endl
		<<"\t                                                                       |     Backspace|"				<<endl
		<<"\t                                                                       '--------------'"				<<endl
		<<"\t      _____                                                                ___________"				<<endl
		<<"\t     |     |                                                              |      Enter|"				<<endl
		<<"\t     |   W |                                                              |       <---|"				<<endl
		<<"\t     '-----'                                                              '--.        |"				<<endl
		<<"\t _____  _____  _____                                                         |        |"				<<endl
		<<"\t|     ||     ||     |                                                        |        |"				<<endl
		<<"\t|   A ||   S ||   D |                                                        |        |"				<<endl
		<<"\t'-----''-----''-----'                                                        '--------'"				<<endl
		<<"\t                                                                                     _____"			<<endl
		<<"\t                                                                                    |     |"			<<endl
		<<"\t                                                                                    |   ^ |"			<<endl
		<<"\t                                                                                    '-----'"			<<endl
		<<"\t                      _________________________________________________       _____  _____  _____"		<<endl
		<<"\t                     |                                                 |     |     ||     ||     |"	<<endl
		<<"\t                     |                                                 |     |   < ||   v ||   > |"	<<endl
		<<"\t                     '-------------------------------------------------'     '-----''-----''-----'";
    }
    else{
	cout<<"\t                                                                                     Valid Keys"		<<endl
		<<"\t      _____"																							<<endl
		<<"\t     |     |"																							<<endl
		<<"\t     |   W |"																							<<endl
		<<"\t     '-----'"																							<<endl
		<<"\t _____  _____  _____"																					<<endl
		<<"\t|     ||     ||     |"																					<<endl
		<<"\t|   A ||   S ||   D |"																					<<endl
		<<"\t'-----''-----''-----'"																					<<endl
		<<"\t                                                                                     _____"			<<endl
		<<"\t                                                                                    |     |"			<<endl
		<<"\t                                                                                    |   ^ |"			<<endl
		<<"\t                                                                                    '-----'"			<<endl
		<<"\t                                                                              _____  _____  _____"		<<endl
		<<"\t                                                                             |     ||     ||     |"	<<endl
		<<"\t                                                                             |   < ||   v ||   > |"	<<endl
		<<"\t                                                                             '-----''-----''-----'";
    }
    cout<<endl<<endl<<endl<<"\t\t\t\t\t\tPress Enter to Start!";
}
void ExibeNomeMapa()				//Exibe a escolha do nome
{
	clearScreen();
	cout<<endl<<endl<<endl<<endl;
	cout<<"\t\t\t _"								    <<endl
		<<"\t\t\t/ ` _ _  _ _|_ _    _   |\\/| _  _"    <<endl
		<<"\t\t\t\\_,| (/_(_| | (/_  (_|  |  |(_||_)"   <<endl
        <<"\t\t\t                               |";
    cout<<endl<<endl<<endl<<endl<<endl<<endl<<"\t\t\tName your Map: ";
	for (int i=0; i<my; i++){
		for (int j=0; j<mx; j++){
				 if((i==0) &&(j==0)  ){ mapa.map[i][j]=201; }//Canto Superior Esquerdo
			else if((i==0) &&(j==118)){ mapa.map[i][j]=187; }//Cando Superior Direito
			else if((i==29)&&(j==0)  ){ mapa.map[i][j]=200; }//Canto Inferior Esquerdo
			else if((i==29)&&(j==118)){ mapa.map[i][j]=188; }//Canto Inferior Direito
			else if((j==0) ||(j==118)){ mapa.map[i][j]=186; }//Laterais do Mapa
			else if((i==0) ||(i==29) ){ mapa.map[i][j]=205; }//Chao e Topo do Mapa
			else					  { mapa.map[i][j]=  0; }//Vazio
		}
	}
}
void proximomapa()										//Muda para o proximo mapa
{
	mapa.path="";
	if(jogador.mapa==1){ CarregaMapa("map1.txt",mapa.path); mapa.name="map1"; }
	if(jogador.mapa==2){ CarregaMapa("map2.txt",mapa.path); mapa.name="map2"; }
	if(jogador.mapa==3){ CarregaMapa("map3.txt",mapa.path); mapa.name="map3"; }
	if(jogador.mapa==4){ CarregaMapa("map4.txt",mapa.path); mapa.name="map4"; }
	if(jogador.mapa==5){ GameRunning=false; LimpaTela(); }
	if(jogador.mapa==0){ GameRunning=false; LimpaTela(); }
	TempoIni=GetTickCount();
}
void ExibeGameWon()				//Quando voce passa de fase
{
	clearScreen();
	cout<<endl<<endl<<endl<<endl;
	color.set(text_color::YELLOW);
	cout<<"\toooooo   oooo                          oooooo   oooooo     oooo"							<<endl
		<<"\t `888.   .8'                            `888.    `888.     .8'"							<<endl
		<<"\t  `888. .8'    .ooooo.  oooo  oooo       `888.   .8888.   .8'    .ooooo.  ooo. .oo."		<<endl
		<<"\t   `888.8'    d88' `88b `888  `888        `888  .8'`888. .8'    d88' `88b `888P'Y88b"		<<endl
		<<"\t    `888'     888   888  888   888         `888.8'  `888.8'     888   888  888   888"		<<endl
		<<"\t     888      888   888  888   888          `888'    `888'      888   888  888   888"		<<endl
		<<"\t    o888o     `Y8bod8P'  `V88V'V8P'          `8'      `8'       `Y8bod8P' o888o o888o";
    cout<<endl<<endl<<"\t\t\tTempo recorde: "<<mapa.RecordAtual
			  <<endl<<"\t\t\t    Seu tempo: "<<mapa.tempo
			  <<endl; 
			  if(mapa.tempo<=mapa.RecordAtual){
			  	cout<<"\t\t\tParabens, voce bateu o recorde anterior!!!";
			  }
	cout<<endl<<endl<<"\t\t\t\t\t\tPress Enter to Exit!";
}
void ExibeGameOver()				//Exibe a tela de Fim de Jogo
{
	clearScreen();
	cout<<endl<<endl<<endl<<endl;
	color.set(text_color::LIGHTRED);
	cout<<"\t  .oooooo.                                               .oooooo."									<<endl
 		<<"\t d8P'  `Y8b                                             d8P'  `Y8b"								<<endl
		<<"\t888            .oooo.   ooo. .oo.  .oo.    .ooooo.     888      888 oooo    ooo  .ooooo.  oooo d8b"<<endl
		<<"\t888           `P  )88b  `888P'Y88bP'Y88b  d88' `88b    888      888  `88.  .8'  d88' `88b `888''8P"<<endl
		<<"\t888     ooooo  .oP'888   888   888   888  888ooo888    888      888   `88..8'   888ooo888  888"	<<endl
		<<"\t`88.    .88'  d8(  888   888   888   888  888    .o    `88b    d88'    `888'    888    .o  888"	<<endl
		<<"\t `Y8bood8P'   `Y888''8o o888o o888o o888o `Y8bod8P'     `Y8bood8P'      `8'     `Y8bod8P' d888b";
    cout<<endl<<endl<<endl<<endl<<endl<<endl<<"\t\t\t\t\t\tPress Enter to Exit!";
}
void listadir ()
{
	DIR *dpdf;
	struct dirent *epdf;

	int c=-2;

	dpdf = opendir(".\\CustomMaps");
	if (dpdf != NULL){
		while (epdf = readdir(dpdf)){
	   		if(c>=0){mapascustom[c]=epdf->d_name;}
	   		c++;
		}
	}
}
void CustomMenu()
{
	clearScreen();
	cout<<endl<<endl;
	cout<<"\t\t\t _"									<<endl
		<<"\t\t\t(  _ | _  __|_  _|_|_  _   |\\/| _  _"	<<endl
		<<"\t\t\t_)(/_|(/_(_ |    | | |(/_  |  |(_||_)" <<endl
		<<"\t\t\t                                  |";
	for(int i=0; i<25; i++){
		cout<<endl<<"\t\t\t";
		if(i==customopcao-2){ color.set(text_color::LIGHTRED); cout<<direita<<" "<<mapascustom[i]; }else{ color.reset(); cout<<"  "<<mapascustom[i]; }
	}
}
void jogadormorre()										//Morre ou ganha
{
	if((mapa.map[jogador.y][jogador.x]==princes)||(mapa.map[jogador.y][jogador.x]==chaopri)){//Ganha
		jogador.vidas++;
		if(jogador.mapa!=0){ jogador.mapa++; }
		LimpaTela();
		GameWon=true;
		if(mapa.tempo<=mapa.RecordAtual){
			mapa.Record=mapa.tempo;
			SalvaMapa(mapa.name,mapa.path);
		}
		if(mapa.RecordAtual==0){
			mapa.Record=mapa.tempo;
			SalvaMapa(mapa.name,mapa.path);
		}
		GameRunning=false;
	}
	for(int a=0; a<BAR_QTD; a++){					//Morre
		if(((barril[a].y==jogador.y)&&(barril[a].x==jogador.x))||(mapa.map[jogador.y][jogador.x]==chao)){
			jogador.x=jogador.sx;
			jogador.y=jogador.sy;
			jogador.vidas--;
			for(int b=0; b<BAR_QTD; b++){barril[b].x=dk.x; barril[b].y=dk.y;}
			break;
		}
	}
	if(jogador.vidas==0){ jogador.mapa=1; GameRunning=false; GameOver=true; LimpaTela(); } //Quando fica sem vidas
}
void up(char* menu)
{
		 if(menu=="Game"){ //Se o jogo estiver em execucao
			if(mapa.map[jogador.y][jogador.x]!=0  ) //Sobe se nao for | ou espaco
			if(mapa.map[jogador.y][jogador.x]!=124){//Sobe se nao for | ou espaco
				jogadormorre();
				jogador.y--;
				jumptime=EXEC_TIME;
				if((mapa.map[jogador.y][jogador.x]!=45)&&(mapa.map[jogador.y+1][jogador.x]!=45)){ jogador.pulo=1; }
				jogadormorre();
			}
		 }
	else if(menu=="Creator"){ 	//Criador
			if(cursor.y!=1){cursor.y--;}
			if((CreatorRunning!=2)&&(CreatorRunning!=1)){ CreatorRunning--; }
		 }
	else if(menu=="CustomMenu"){if((customopcao!=2)&&(customopcao!=1)){ customopcao--; }}
	else if(menu=="Principal" ){if(( AppRunning!=2)&&( AppRunning!=1)){  AppRunning--; }}
}
void down(char* menu)
{
		 bool desceu = false;									//Bool pra nao descer duas casas se estiver na escada
		 if(menu=="Game"){ //Se o jogo estiver em execucao
			if(mapa.map[jogador.y][jogador.x]!=95)//underline		//Desce se puder
			if(mapa.map[jogador.y][jogador.x]!=esquerda)// «
			if(mapa.map[jogador.y][jogador.x]!=direita) // »
				jogadormorre();
				jogador.y++;
				desceu=true;
				jogadormorre();
			}
			if((mapa.map[jogador.y+1][jogador.x]==45)&&(desceu!=true)){	//sem o "desceu" ele desce duas casas se apertar
				jogador.y++;										    //para baixo quando esta no meio da escada
				jogadormorre();
			}
	else if(menu=="Creator"){   //Criador
			if(cursor.y!=28){cursor.y++;}
			if((CreatorRunning!=4)&&(CreatorRunning!=1)){ CreatorRunning++; }
		 }
	else if(menu=="CustomMenu"){if((customopcao!=26)&&(customopcao!=1)){customopcao++; }}
	else if(menu=="Principal" ){if(( AppRunning!=5)&&( AppRunning!=1)){  AppRunning++; }}
}
void right(char* menu)
{
		 if(menu=="Game"){ //Se o jogo estiver em execucao
			if(mapa.map[jogador.y][jogador.x+1]!=parede){//Vai pra direita
				jogadormorre();
				jogador.x++;
				jogadormorre();
			}
		 }
	else if(menu=="Creator"){ if(cursor.x!=117){cursor.x++;} } //Se o menu estiver na tela
}
void left(char* menu)
{
		 if(menu=="Game"){ //Se o jogo estiver em execucao
			if(mapa.map[jogador.y][jogador.x-1]!=parede){//Vai pra esquerda
				jogadormorre();
				jogador.x--;
				jogadormorre();
			}
		 }
	else if(menu=="Creator"){ if(cursor.x!=1){cursor.x--;} } //Se o criador estiver na tela
}
void ESC(char* menu)
{
		 if(menu=="Game"      ){       GameRunning=0; LimpaTela();}	//Se o jogo estiver em execucao
	else if(menu=="Creator"   ){	CreatorRunning=2; LimpaTela();}	//Se o criador estiver executando
	else if(menu=="Keyboard"  ){       KeyRunning =0; LimpaTela();}	//Se o teclado estiver na tela
	else if(menu=="CustomMenu"){       customopcao=0; LimpaTela();}	//Se menu custom
	else if(menu=="GameWon"   ){       GameWon=false; LimpaTela();} //Ganhou parça \o
	else if(menu=="Principal" ){									//Se o menu estiver na tela
			if(AppRunning!=1){ AppRunning=1; }else{ AppRunning=0; }
			if(AppRunning!=0){	LimpaTela(); }
		}
}
void enter(char* menu)
{
		 if(menu=="Game"     ){}	//Se o jogo estiver em execucao
	else if(menu=="Keyboard" ){		//Se o teclado estiver na tela
			KeyRunning=false;
			if(AppRunning==2){ GameRunning=true; jogador.vidas=3; jogador.mapa=1; proximomapa(); }//Start Game
			if(AppRunning==3){ customopcao=2; LimpaTela(); listadir(); TempoIni=GetTickCount(); }//Play a Custom Map
			if(AppRunning==4){ CreatorRunning=1; cursor.y=1; cursor.x=1; LimpaTela(); jogador.sx=1; jogador.sy=1; }//Create a Map
			LimpaTela();
		}
	else if(menu=="Creator" ){//Criador
	        mapa.path=".\\CustomMaps\\";
	        mapa.Record=0;
				 if(CreatorRunning==2){
				 for (int i=0; i<my; i++){for (int j=0; j<mx; j++){if(mapa.map[i][j]==80){jogador.sx=j;jogador.sy=i;}}}
										SalvaMapa(mapa.name,mapa.path); CreatorRunning=0; LimpaTela(); }//Salva Mapa
			else if(CreatorRunning==3){									CreatorRunning=0; LimpaTela(); }//Nao Salva
			else if(CreatorRunning==4){									CreatorRunning=1; LimpaTela(); }//Cancela

			else if(mapa.map[cursor.y][cursor.x]==0       ){ mapa.map[cursor.y][cursor.x]=esquerda; }//Muda as casas do mapa
			else if(mapa.map[cursor.y][cursor.x]==esquerda){ mapa.map[cursor.y][cursor.x]=direita;  }
			else if(mapa.map[cursor.y][cursor.x]==direita ){ mapa.map[cursor.y][cursor.x]=troca;    }
			else if(mapa.map[cursor.y][cursor.x]==troca   ){ mapa.map[cursor.y][cursor.x]=barra;    }
			else if(mapa.map[cursor.y][cursor.x]==barra   ){ mapa.map[cursor.y][cursor.x]=45;       }
			else if(mapa.map[cursor.y][cursor.x]==45      ){ mapa.map[cursor.y][cursor.x]=chaopri;  }
			else if(mapa.map[cursor.y][cursor.x]==chaopri ){ mapa.map[cursor.y][cursor.x]=80;       }
			else if(mapa.map[cursor.y][cursor.x]==80      ){ mapa.map[cursor.y][cursor.x]=33;       }
			else if(mapa.map[cursor.y][cursor.x]==33      ){ mapa.map[cursor.y][cursor.x]=princes;  }
			else if(mapa.map[cursor.y][cursor.x]==princes ){ mapa.map[cursor.y][cursor.x]=0;        }
		}
	else if(menu=="GameWon" ){	//Ganhou parça \o
			GameWon=false;
			LimpaTela();
			if((jogador.mapa!=0)&&(jogador.mapa!=5)){ proximomapa(); GameRunning=true; }
		}
	else if(menu=="GameOver" ){	//Morreu demais parca
			GameOver=false;
			LimpaTela();
		}
	else if(menu=="CustomMenu"){//Menu de mapas custom
			//Remover .txt do nome
			mapa.name=mapascustom[customopcao-2];
			mapa.name.erase (mapa.name.end()-4,mapa.name.end());
			//
			mapa.path=".\\CustomMaps\\";
			CarregaMapa(mapascustom[customopcao-2],mapa.path);
			GameRunning=true;
			customopcao=0;
			jogador.mapa=0;
			jogador.vidas=3;
			TempoIni=GetTickCount();
			LimpaTela();
		}
	else if(menu=="Principal"){		//Se o menu estiver na tela
				 if(AppRunning==1){ AppRunning=2; }		//Press Enter to continue
			else if(AppRunning==5){ AppRunning =   0; }	//Exit
			else { KeyRunning=true;	}
			if(AppRunning!=0){LimpaTela();}
		}
}
void back(char* menu)
{
		 if(menu=="Game"     ){}	//Se o jogo estiver em execucao
	else if(menu=="Keyboard" ){}		//Se o teclado estiver na tela
	else if(menu=="Creator"  ){//Criador
				 if(mapa.map[cursor.y][cursor.x]==0       ){ mapa.map[cursor.y][cursor.x]=princes;  }//Muda as casas do mapa
			else if(mapa.map[cursor.y][cursor.x]==princes ){ mapa.map[cursor.y][cursor.x]=33;		}
			else if(mapa.map[cursor.y][cursor.x]==33      ){ mapa.map[cursor.y][cursor.x]=80;       }
			else if(mapa.map[cursor.y][cursor.x]==80      ){ mapa.map[cursor.y][cursor.x]=chaopri;  }
			else if(mapa.map[cursor.y][cursor.x]==chaopri ){ mapa.map[cursor.y][cursor.x]=45;       }
			else if(mapa.map[cursor.y][cursor.x]==45      ){ mapa.map[cursor.y][cursor.x]=barra;    }
			else if(mapa.map[cursor.y][cursor.x]==barra   ){ mapa.map[cursor.y][cursor.x]=troca;    }
			else if(mapa.map[cursor.y][cursor.x]==troca   ){ mapa.map[cursor.y][cursor.x]=direita;  }
			else if(mapa.map[cursor.y][cursor.x]==direita ){ mapa.map[cursor.y][cursor.x]=esquerda; }
			else if(mapa.map[cursor.y][cursor.x]==esquerda){ mapa.map[cursor.y][cursor.x]=0;        }
		}
	else if(menu=="GameOver" ){}//Morreu demais parca
	else if(menu=="CustomMenu" ){}	//Menu de mapas custom
	else if(menu=="Principal"){}	//Se o menu estiver na tela
}
void pulando(){											//Cai e pulo
	jogadormorre();
		 if(jogador.pulo==1    ){ jogador.y--; jogador.pulo=0; }
	else if(mapa.map[jogador.y][jogador.x]==0  ){ jogador.y++; }
	else if(mapa.map[jogador.y][jogador.x]==124){ jogador.y++; }
	jogadormorre();
}
void funcoes()
{
	for(int a=0; a<BAR_QTD; a++){
			 if(mapa.map[barril[a].y][barril[a].x]==0       ){ barril[a].y++; }//Barril cai se der
		else if(mapa.map[barril[a].y][barril[a].x]==direita ){ barril[a].x++; }//Barril vai pra direita
		else if(mapa.map[barril[a].y][barril[a].x]==esquerda){ barril[a].x--; } //Barril vai pra esquerda
		else if(mapa.map[barril[a].y][barril[a].x]==95      ){barril[a].x=dk.x; barril[a].y=dk.y;}//Barril reseta se chega on tem um "_"(underline)
		else if(mapa.map[barril[a].y][barril[a].x]==chao    ){barril[a].x=dk.x; barril[a].y=dk.y;}//Barril reseta se chega on tem um "-"(chao)
		jogadormorre();
	}
}
void CriarBarril()	         //Cria Barril
{
	for(int a=0; a<BAR_QTD; a++){
		if((barril[a].x==dk.x)&&barril[a].y==dk.y){
			barril[a].y++;
			break;
		}
	}
}

void CapturaTecla(char* menu)	//Funcao loca do capiroto pra ler a tecla que voce aperta, sem parar a aplicacao
{
	HANDLE rhnd = GetStdHandle(STD_INPUT_HANDLE);  //handle para ler o console
    DWORD Events = 0;     //Contar eventos
    DWORD EventsRead = 0; //Eventos do console
	GetNumberOfConsoleInputEvents(rhnd, &Events);
    if(Events != 0){//if something happened we will handle the events we want
        INPUT_RECORD eventBuffer[Events];//create event buffer the size of how many Events
		ReadConsoleInput(rhnd, eventBuffer, Events, &EventsRead);//fills the event buffer with the events and saves count in EventsRead
        for(DWORD i = 0; i < EventsRead; ++i){//loop through the event buffer using the saved count
            if(eventBuffer[i].EventType == KEY_EVENT && eventBuffer[i].Event.KeyEvent.bKeyDown){//check if event[i] is a key event && if so is a press not a release
				switch(eventBuffer[i].Event.KeyEvent.wVirtualKeyCode){//check if the key press was an arrow key
		            case VK_LEFT:	left(menu); 			break;
		            case VK_RIGHT:	right(menu);			break;
		            case VK_UP:		up(menu);				break;
		            case VK_DOWN:   down(menu);				break;
		            case 'A': 		left(menu); 			break;
		            case 'D': 		right(menu);			break;
		            case 'W': 		up(menu);				break;
		            case 'S': 		down(menu);				break;

		            case VK_ESCAPE:	ESC(menu);				break;
		            
		            case VK_BACK  :	back(menu);				break;

		            case VK_RETURN:	enter(menu);			break;
		            case VK_SPACE :	enter(menu);			break;

		            default:								break;
		    	}
            }
        }
    }
}

int main(int argc, char** argv)
{
	//Muda o tamanho do console
	    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        SMALL_RECT sr;
        COORD consoleSize;

        consoleSize.X = 120; consoleSize.Y = 31;

        sr.Top=sr.Left=0;
        sr.Right=120; sr.Bottom=31;
    //
	int exectime=EXEC_TIME, bartime=BAR_TIME;//Alguns temporizadores
	bool showkey=false,showgo=false;//Nao quero que a tela fique atualizando quando mostra o teclado e gameover

	while(AppRunning!=0){
		ExibeMenu();
		CapturaTecla("Principal");
		showkey=false;showgo=false;//Reseta

		//Teclas disponiveis
		while(KeyRunning){
			if(showkey){}else{ ExibeTeclado(); showkey=true; }
			CapturaTecla("Keyboard");
		}
		//
		//Menu de Mapas customisados
		while(customopcao!=0){
			CustomMenu();
			CapturaTecla("CustomMenu");
		}
		//
		//Execucao do Jogo
		while(GameRunning){
			ExibeMapa();
			if(jumptime>0){jumptime--;}else{pulando();   jumptime=EXEC_TIME;}	//Chama a queda do personagem em determinados tempos
			if(exectime>0){exectime--;}else{funcoes();   exectime=EXEC_TIME;}	//Chama todas as funcoes do programa (barris andar e cair)
			if(bartime>0) { bartime--;}else{CriarBarril(); bartime=BAR_TIME;}	//Cria um barril
			CapturaTecla("Game");
			mapa.tempo=GetTickCount()-TempoIni;
		};
		//
		//Criacao de Mapa
		while(CreatorRunning!=0){
			if(showgo){}else{ ExibeNomeMapa(); showgo=true; getline(cin,mapa.name); }
			ExibeCriador();
			CapturaTecla("Creator");
		}
		//
		//GameWon
		while(GameWon){
			ExibeGameWon();
			CapturaTecla("GameWon");
		}
		//
		//GameOver
		while(GameOver){
			if(showgo){}else{ ExibeGameOver(); showgo=true; }
			CapturaTecla("GameOver");
		}
		//
	};
	return 0;
}
