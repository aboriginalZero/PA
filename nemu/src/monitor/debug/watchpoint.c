#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	WP *p=free_;
	if(!free_) assert(0);
	free_=free_->next;
	if(!head){
		head=p;
		p->NO=1;
	}else{
		int cnt=2;
		WP *q=head;
		while(q->next){
			cnt++;
			q=q->next;
		}
		p->NO=cnt;
		q->next=p;
		p->next=NULL;
	}
	return p;
}
void free_wp(WP *wp){
	WP* p=head;
	WP* q=head;
	if(q==wp){//头结点
		head=head->next;
	}else{
		while(q!=wp){	
			p=q;
			q=q->next;
		}
		p->next=q->next;
	}
	if(!free_){
		free_=wp;
	}else{
		wp->next=free_;
		free_=wp;
	}
}
void createWatchPoint(char *args){
	bool *flag=false;
	WP *p=new_wp();
	strcpy(p->expr,args);
	int value=expr(p->expr,flag);
	p->type=1;
	if(args[0]=='$'){
		p->value=vaddr_read(value,4);
		printf("ssss\n");
	}
		
	else 
		p->value=value;
}
WP* searchWatchPoint(int num){
	WP *p=head;
	num--;
	while(num--){
		if(!p){
			printf("NO.%d is not exist!\n",num);
			return NULL;
		}
		p=p->next;
	}
	return p;
}
bool judgeWatchPoint(){
	bool flag;
	bool *temp=false;
	int value;
	WP *p=head;
	while(p){
		value=expr(p->expr,temp);
		if(value!=p->value){
			printf("NO.%d\toldValue:0x%xnewValue:0x%x\n",p->NO,p->value,value);
			p->value=value;
			flag=true;
		}
		p=p->next;
	}
	return flag;
}
void printAllWatchPoint(){
	WP *p=head;
	while(p){
		printf("Num	Type		expr	value\n");
		printf("%d	%d	%s	0x%x\n", p->NO,p->type,p->expr,p->value);		
		p=p->next;
	}
}



