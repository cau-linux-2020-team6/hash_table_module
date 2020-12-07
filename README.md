# hash_table_module

리눅스 커널. 해시테이블에서 키가 서로 다른 데이터를 넣고자 할 때,

키가 서로 다르더라도 테이블이 작거나, 어떠한 우연으로 인해 테이블의 하나의 엔트리에 값이 뭉치게 되면

해시테이블의 강점인 상수시간 내의 탐색이 불가능하다. N개의 데이터가 하나의 엔트리로 몰리게 되면, 탐색 시간은 O(N)으로 커지는 것이다.

물론, 애초에 해시테이블을 크게 잡았거나, 운이 좋다면야 피할 수 있는 문제이지만.

데이터가 뭉치더라도 어느정도의 속도를 보장할 방법이 없을까?

처음에는 먼저 rbtree를 이용해서 key가 다르지만 같은 칸으로 응집되는 경우에 대비하고, 키도 같은 경우에만 링크드 리스트를 쓰게 하고 싶었다.

즉, 각 테이블 엔트리가 트리의 최상단 노드가 되고, 트리의 각 노드마다 링크드리스트가 연결이 되어 있는 형태로.

하지만, 아쉽지만 이는 부족한 경험으로 인해 삽질만을 불러일으켰고, 링크드리스트를 포기해야 했다.

현재의 구성에서 이미 동일한 키값이 주어져 있는 데이터는 무시된다. 암호화 프로토콜 등을 이용해 키의 분포를 넓게 퍼뜨리는 것도 방법이 될 수 있을 것이다.

# 헤더의 구조와 구현 범위

 ## hashtable_noduplicate.h
 
 linux/hashtable.h를 수정해서 만든 헤더파일이다.
 
 struct hrb_node를 정의했다. 기본적으로는, 기존에 이미 존재하는 rbtree를 사용하는 형태가 된다.
 
    struct hrb_node{
        struct rb_node node;
        u32 key;
    }
 node는 rb node를, key는 hashtable에 삽입하는데 사용된 키를 의미한다. key는 rbtree를 정렬하는데도 사용된다.
 
 아래는 지원 함수의 목록이다.
 
     #define DEFINE_HASHTREE(name, bits)
         //새로운 hash rbtree를 생성하고, 초기화함.
     #define DECLARE_HASHTABLE(name, bits)
         //새로운 hash rbtree를 생성하되, 초기화는 하지 않음.
     #define hashrbtree_init(hashtable)
         //이미 생성된 해시 테이블을 초기화.
     void hashrbtree_add(struct rb_root* hashtable, struct hrb_node* node, int hash_bits)
        //hashtable에 새로운 노드를 삽입.
    void hashrbtree_del(struct rb_root* node)
        //해시테이블의 인덱스 하나를 붕괴시킴.
    #define bucket_for(name, key)
        &name[hash_min(key, HASH_BITS(name))]
        //특정 키를 가진 노드가 삽입될 테이블의 엔트리의 주소를 가져옴.
    
아래는 데이터 열거에 쓰일만한 매크로 목록이다.

    #define hash_rbtree_for_each(name, bkt, iter)
        @name: 순회할 해시 rb테이블
        @bkt: 해시 rb테이블을 하나씩 순회하기 위한 정수형 임시변수
        @iter: rb_node*형 포인터. rb_entry()를 통해, 포인터가 가리키는 원본 데이터 영역에 접근할 수 있다.
    
    #define hash_rbtree_for_each_possible(name, rptr, key)
        @name: 순회할 해시 rb테이블
        @rptr: rb_node*형 포인터.
        @key: 순회할 키값.
        //키가 정확히 일치하는 데이터 뿐만 아니라, 해시 rb테이블에서 같은 엔트리에 담긴 모든 노드를 가져온다.
        
    #define hash_rbtree_possible(name, key)
        @name: 순회할 해시 rb테이블
        @key: 찾을 키값
        //키가 정확히 일치하는 데이터를 찾는다. 찾는다면 rb_node*형을 반환하고, 찾지 못한다면 NULL을 반환한다.
    
# 테스트용 코드

hashtable.c는 기존 리눅스 커널의 hashtable구조를 이용한다.

hashtable_noduplicate.c는 해당 코드를 수정해서 만든 hash rbtree를 테스트한다.

둘의 구조가 거의 비슷하게끔 설계했지만, 동일하지는 않다. 

# 빌드 방법

리눅스 커널 환경에서 테스트했다. 해당 빌드 에센셜이 없다면 설치해야 한다.

Makefile는 기본적으로 hashtable_noduplicate.ko를 생성한다.

생성 후에는 sudo insmod hashtable_noduplicate.ko로 테스트해볼 수 있다.
