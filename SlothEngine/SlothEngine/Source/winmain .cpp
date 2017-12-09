//Edited 2015-10-14  




#include "..\Headers\Core.h"
#include <string.h>

class Applikation : public Core{
public:
	Applikation(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH);
	~Applikation();

	bool Init() override;


};

Applikation::Applikation(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH) :Core(hInstance, name, scrW, scrH){

}

Applikation::~Applikation(){

}

bool Applikation::Init(){
	if (!Core::Init()){
		return false;
	}

	return true;
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	Applikation* pApplication = new Applikation(hInstance, "SlothEngine", 1024, 1024);

	if (!pApplication->Init()){
		MessageBox(NULL, L"Kunde inte k?ra Init()", NULL, NULL);
		return 1;
	}

	pApplication->Run(); //om allt funkar s? ska den b?rja k?ra applikationen - Run() k?r ig?ng bla Update() och Render()
	return 0;
}