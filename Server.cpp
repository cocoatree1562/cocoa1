//여기서 제가 제일 먼저 볼거
//서버에는 ip가 있기 마련입니다
//저희 내부 네트워크에도 ip를 알려주어야 해요
//내부 ip를 쳐주시면 됩니다
#define SERVER_IP "10.178.0.4"
//동적  포트를 사용해보도록 합시다
#define SERVER_PORT 61255
//너무 많은 양의 전송을 하면 성능상도 그렇구
//물리적인 네트워크 기기에도 한계가 있기 때문에
//버퍼 (임시 저장공간)사이즈는 제한을 걸어줄게요
#define BUFFER_SIZE 1024
//제한을 해야되는 요소는 굉장히 많이 있는데요
//동시 접속자 서버가 원활하게 돌아갈 수 있도록
//접속 인원의 한계를 미리 정해놓습니다
//이 한계를 넘는 인원이 들어오는 경우 대기열 서버로 넘겨줍니다
//왜 FD(File Decriptor)인가! 소켓으로 통신을 하게 될 거에요
//윈도우스 같은 경우에는 소켓이 따로 있습니다
//리녹스는 모든 것을 파일 형태로 관리해요 소켓 조차도 파일이거든요
//그래서 FD 넘버라고 헀어요
#define USER_MAXIMUM 100
//서버가 무한한 속도로 돌아가면 물론 좋겠죠
//서버의 틱레이트를 조절해주실 필요가 있는데요
//클라이언트 같은 경우는 144프레임으로 하시는분 굉장히 많습니다
#define TICK_RATE 16

#include <iostream>
//클라이언트가 직접 주소와 포트를 이용해서 들어오라고 소켓을 사용할 거에요
#include <sys/socket.h>
//IP쓰려고 INet을 가져오도록 할게요
#include <netinet/in.h>
#include <arpa/inet.h>
//플레이어들을 계속 순회하면서 자연스럽게 저한테 뭔가 내용 줄에다가 있는 경우에만 활용하도록 polling을 사용할 겁니다
#include <poll.h>
//틱레이트에 대한 이야기를 했었죠
//얼마나 시간이 지났는지 체크를 하기는 해야 계산을 시작할 수 있을 거에요
#include <sys/time.h>
//자료형을 가지고 놀아볼 예정이거든요? 그래서 타입을 가볍게 가져왔어요
#include <sys/types.h>
//문자열
#include <string.h>
#include <unistd.h>

#include "ServerEnum.h"

using namespace std;

//전방선언//
 
//전방선언//

union FloatUnion
{
	float floatValue;
	char charArray[4];
};

union IntUnion
{
	int intValue;
	char charArray[4];
};

FloatUnion floatChanger;
IntUnion intChanger;

class UserData
{
public:
	//본인이 타고 있는 소켓의번호를 저장해둡니다
	//나중에 애한테 연락해야하는 일이 있을때 유용하게 사용하겠죠
	int FDNumber = 0;
	//목먹지 x,y,z를 넣어줄 거에요
	float destinationX, destinationY, destinationZ;
	//위치 x,y,z를 넣어주기
	float locationX, locationY, locationZ;

	UserData()
	{
		cout << "유저 데이터가 생성되었습니다" << endl;
	}

	~UserData()
	{
		cout << "유저 연결이 종료되었습니다" << endl;
	}
};

//====전역변수 선언란====//
struct pollfd pollFDArray[USER_MAXIMUM];
UserData* userFDArray[USER_MAXIMUM];
//====전역변수 선언란====//


bool StartServer(int* currentFD)
{
	//위에서 못 만들어 왔을 때
	if (*currentFD == -1)
	{
		perror("socket()");
		close(*currentFD);
		return true;
	}

	//소켓에는 ip와 포트가 있어요 넣어주어야 하겠죠?
	sockaddr_in address;

	//0으로 초기화해서 확실하게 합시다
	memset(&address, 0, sizeof(address));

	//IPv4의 IP를 넘겨줄테니깐 그렇게 아세요
	address.sin_family = AF_INET;
	//IP같은 경우는 ff.ff.ff.ff 같이 점이 굉장히 많이 있어서 숫자로 표시하기 애매하니깐
	//보통은 이렇게 문자열로 만든 다음에 함수로 그걸 풀어서 써요
	address.sin_addr.s_addr = inet_addr(SERVER_IP);//0x0AB20005
	//포트도 그대로 가져와주기
	address.sin_port = htons(SERVER_PORT);

	//저희가 정보를 만들어가지고 왔으니깐 정보는 그냥 만들었다과 끝이 아니죠
	//어진가(리슨 소켓)에 연결해야 의미가 있는것이죠
	if (bind(*currentFD, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		//바인드에 에러가 났어요
		perror("bind()");
		close(*currentFD);
		return true;
	}

	//바인드만 하면 끝인가요? 얘가 소켓인것도 알겠고,, IP주소도 알겠고 포트도 알겠는데,
	//사실 다른애들도 가지고있으려면 가질 수 있거든요..?
	//제가 처음에 이 녀석을 만들 때 [리슨 소캣]이라고 했어요
	if (listen(*currentFD, 8) == -1)
	{
		//리슨에 에러가 났어요
		perror("listen()");
		close(*currentFD);
		return true;
	}

	//아무리 문제 없이 다 잘끝났어요
	return false;
}

void CheckMessage(int userNumber,char receive[], int length)
{		//			 맨 앞 1바이트는 메세지 구문용이니깐
	char* value = new char[length - 1];
	try
	{

		//			 맨 앞 1바이트
		memcpy(value, receive + 1, length - 1);
		//이 아래쪽은 받는 버퍼의 내용을 가져왔을 때에만 여기 있겠죠
		//받은 메세지의 0번칸은 메세지의 타입을 정의합니다
		//물론 나중에 255개의 메세지 타입이 부족하다라고 생각하신 경우에는
		//다른 바이트도 같이 확인을 하셔야 하겠지만, 지금은 그냥 바이트 하나만 보면 됩니다
		switch (receive[0])
		{
		case Chat:
			cout << value << endl;

			//0번 리슨포트였죠, 리슨포트에다가 그대로 전달을 해주시면
			//서버가 서버한테 접속시도한 거니깐, 요거는 하지맙시다
			for (int i = 1; i < USER_MAXIMUM; i++)
			{
				//유저가 있다
				if (pollFDArray[i].fd != -1)
				{
					//유저한테 반갑다고 인사해줍시다
					write(pollFDArray[i].fd, receive, length);
				}
			}
			break;

		case Move:
			cout << "플레이어 이동 수신" << endl;

			for (int i = 1; i < 4; i++) floatChanger.charArray[i] = receive[i + 1];
			userFDArray[userNumber]->destinationX = floatChanger.floatValue;
			for (int i = 1; i < 4; i++) floatChanger.charArray[i] = receive[i + 5];
			userFDArray[userNumber]->destinationY = floatChanger.floatValue;
			for (int i = 1; i < 4; i++) floatChanger.charArray[i] = receive[i + 9];
			userFDArray[userNumber]->destinationZ = floatChanger.floatValue;

			for (int i = 1; i < USER_MAXIMUM; i++)
			{
				if (pollFDArray[i].fd != -1)
				{
					write(pollFDArray[i].fd, receive, length - 1);
				}
			}
			break;
		}
	}
	catch (exception& e)
	{

		cout << e.what() <<endl;
	}
	//value는 다 썼으니깐, 지워주기
	delete[] value;
}



int main()
{
	
	//소켓들은 전부다 int로 관리될 거에요 함수를 통해서 접근할 거니깐 너무 걱정하실 필요 없어요
	//사실 컴퓨터의 연결이라고 하는 건 생각보다 까다롭습니다
	//컴퓨터가 내용을 받아주려고 한다면 상대방의 메세지를 받을 준비가 되어있어야 합니다
	//안그러면 그냥 지나가던 이상한 정보를 받을 수 도 있고, 해커의공격이 담긴 메세지를 받을 수도 있어요
	//보통은 소켓이 닫혀있어요 무엇을 받든 무시하는거에요 (열리게 되는 조건이 있어요)
	//제가 이미 그 주소로 메세지를 보냈다면 소켓이 받아줍니다
	//소켓을 열어주는 소켓이 필요한거죠 소켓 하나를 [리슨 소켓]으로 만듭니다
	//"접속 요청"만 받아주는 소켓을 여는 거에요 누가 접속 요청을 한다면 비어있는 소켓 하나를 찾아요 그래서 걔랑 연결시켜주는
	//하나의 창구가 되는거에요
				//IPv4로 연결을 받는다, 연결을 계속 지속한다 
	int listenFD = socket(AF_INET, SOCK_STREAM, 0);
	//연결할 FD
	int connectFD;
	//연결 결과 저장
	int result = 0;

	struct sockaddr_in listenSocket, connectSocket;
	socklen_t addressSize;

	//받는 버퍼
	char buffRecv[BUFFER_SIZE];
	//주는 버퍼
	char buffSend[BUFFER_SIZE];

	//일단 0으로 초기화
	memset(buffRecv, 0, sizeof(buffRecv));
	memset(buffSend, 0, sizeof(buffSend));

	//서버를 시작합니다			실패하면 그대로 프로그램을 종료합시다
	
	if (StartServer(&listenFD)) return -4;

	cout << "서버가 정상적으로 실행되었습니다." << endl;

	//pollFDArray가 제가 연락을 기다리고 있는 애들이에요
	//그러다 보니깐 일단 처음에는 연락해줄 애가 없다는 것을 확인해야겠죠
	for (int i = 0; i < USER_MAXIMUM; i++)
	{
		//-1이 없다는 뜻
		pollFDArray[i].fd = -1;
	}

	//리슨 소캣도 따로 함수 만들어서 돌릴 건 아니니깐
	pollFDArray[0].fd = listenFD;
	//읽기 대기중 지금 가져왔어요
	pollFDArray[0].events = POLLIN;
	pollFDArray[0].revents = 0;

	//무한 반복
	for (;;)
	{
		result = poll(pollFDArray, USER_MAXIMUM, -1);

		if (result > 0)
		{
			//0번이 리슨 소켓이었습니다
			//0번에 들어오려고 하는 애들을 체크해주긴 해야해요
			if (pollFDArray[0].revents == POLLIN)
			{
				//들어오세요
				connectFD = accept(listenFD, (struct sockaddr*)&connectSocket, &addressSize);

				//0번은 리슨소켓이니깐 1번부터 찾아봅시다
				for (int i = 1; i < USER_MAXIMUM; i++)
				{
					//여기 있네
					if (pollFDArray[i].fd == -1)
					{
						pollFDArray[i].fd = connectFD;
						pollFDArray[i].events = POLLIN;
						pollFDArray[i].revents = 0;

						char message[5];
						message[0] = Join;
						intChanger.intValue = i;
						for (int k = 0; k < 4; k++) message[k + 1] = intChanger.charArray[k];

						for (int j = 1; j < USER_MAXIMUM; j++)
						{

							if (pollFDArray[j].fd != -1) write(pollFDArray[j].fd, message, 5);              
						}
						//새로운 유저 정보를 생성합니다
						userFDArray[i] = new UserData();
						//너가 이 자리에 있는거야
						userFDArray[i]->FDNumber = i;

						break;
					};
				};
			};

			//0번은 리슨 소켓이니깐 위에서 처리했으니깐
			//1번부터 돌아주도록 하겠습니다
			for (int i = 1; i < USER_MAXIMUM; i++)
			{
				//이녀석이 저한테 무슨 내용을 전달을 해줬는지 보러갑시다
				switch (pollFDArray[i].revents)
				{
				//암말도 안했어요 그럼무시
				case 0: break;
				//뭔가 말할 때가 있겠죠
				case POLLIN:
					//보낼때는 write였는데, 받아올 때에는 read가 되겠죠
					//받는 용도의 버퍼를 사용해서 읽어주도록 합시다
					//버퍼를 읽어봤는데 아무것도 들어있지 않네요
					//소름이 돋는군요 클라이언트가 뭔가 말을 하였는데
					//열어봤는데 빈 봉투다?
					//이 상황은 클라이언트가 "연결을 끊겠다"라는 의미 입니다
					if (read(pollFDArray[i].fd, buffRecv, BUFFER_SIZE) < 1)
					{
						delete userFDArray[i];
						pollFDArray[i].fd = -1;

						char message[5];
						message[0] = Exit;
						intChanger.intValue = i;
						for (int k = 0; k < 4; k++) message[k + 1] = intChanger.charArray[k];

						for (int j = 1; j < USER_MAXIMUM; j++)
						{

							if (pollFDArray[j].fd != -1) write(pollFDArray[j].fd, message, 5);              
						}

						break;
					};

					//메세지를 해석해봅시다
					//일반적인 채팅은 그렇게까지 막 돌려서 표현하진 않을꺼에요
					//이동 명령이나, 공격명령,인벤토리 사용같은 글자를 활용하는것이 아니라
					//숫자나 그런 쉽게 눈에 보이지 않는 내용을 처리할 때에는
					//
					CheckMessage(i, buffRecv, BUFFER_SIZE);
					break;
				}
			}
			//버퍼를 초기화 시켜주고 가도록 합시다
			memset(buffRecv, 0, BUFFER_SIZE);
		};
	};

	return -4;
}
