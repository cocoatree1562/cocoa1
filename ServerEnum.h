//define이라고 하는 애는 그냥 매크로라고 생각하기 쉽지만
//define이 되어있는지 여부도 확인을 할 수 있어요
//ifdef ifndef으로 확인하시는 겁니다
//한번도 define이 된 적이 없다라고 한다면
//이 헤더파일을 여기서 처음으로 접근 시도한 것이다라고 생각하실 수 있습니다
//그래서 그 때에는 헤더파일 아쪽에 있는 모든 내용을 실행을 하게끔 해주는 것이구요
//이미 한번 헤터파일을 불러온 적이 있었다고 한다면
//define이 되어있었겠죠 그러면 endif로 바로 이동하게 될 것인데
//endif는 헤더의 맨 마지막 끝내버리는 것이니깐
//두번째 define시도부터는 아무것도 하지 않고 넘어가기
//아까 보셨던 싱글톤 비슷하게 구현한 거랑 같은 느낌으로 보시면 되겠습니다
//#pragma once랑 똑같은 역할을 합니다
#ifndef SERVER_ENUM
#define SERVER_ENUM
//							 1		2	  4     8
typedef enum MessageType : char // short int double >> 크기는 나중에 필요하신대로
{
	Chat
}MessageType;


#endif

