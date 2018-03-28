#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,LMOVE=255,RMOVE=254,BE=253,SE=252,TK_EQ=251,
  TK_FEQ=250,TK_NUM_16=249,TK_NUM_8=248,TK_NUM_10=247,TK_REG=246,
	TK_NAG=245,DEREF=244,
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
	{"\\(",'('},{"\\)",')'},
	{"\\$",'$'},
	{"\\*",'*'},{"/",'/'},{"%",'%'},
	{"\\+", '+'},{"-",'-'},
	{"<<",LMOVE},{">>",RMOVE},
	{">",'>'},{">=",BE},{"<",'<'},{"<",SE},
	{"==", TK_EQ},{"!=",TK_FEQ},
	{"&&",'&'},{"\\|\\|",'|'},
	{"0x[0-9a-fA-F]{1,8}",TK_NUM_16},{"0[0-7]{1,8}",TK_NUM_8},{"[0-9]{1,10}",TK_NUM_10},
	{"[a-z]{1,10}",TK_REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
						break;
					case '(':
            tokens[nr_token++].type='(';break;
					case ')':
            tokens[nr_token++].type=')';break;
					case '$':
            tokens[nr_token++].type='$';break;
          case '*':
            tokens[nr_token++].type='*';break;
					case '/':
            tokens[nr_token++].type='/';break;				
					case '%':
            tokens[nr_token++].type='%';break;
					case '+':
            tokens[nr_token++].type='+';break;  
					case '-':
            tokens[nr_token++].type='-';break;
					case LMOVE:
						tokens[nr_token++].type=LMOVE;break;
					case RMOVE:
						tokens[nr_token++].type=RMOVE;break;
					case '>':
            tokens[nr_token++].type='>';break;
					case '<':
            tokens[nr_token++].type='>';break;  
					case BE:
						tokens[nr_token++].type=BE;break;
						case SE:
						tokens[nr_token++].type=SE;break;
          case TK_EQ:
            tokens[nr_token++].type=TK_EQ;break;
          case TK_FEQ:
            tokens[nr_token++].type=TK_FEQ;break;
          case '&':
            tokens[nr_token++].type='&';break;
					case '|':
            tokens[nr_token++].type='|';break;
					case TK_NUM_16:
            tokens[nr_token++].type=TK_NUM_16;break;
					case TK_NUM_8:
            tokens[nr_token++].type=TK_NUM_8;break;
					case TK_NUM_10:
            tokens[nr_token++].type=TK_NUM_10;break;
					case TK_REG:
            tokens[nr_token++].type=TK_REG;break;

					// case TK_NUM_8:
          //   tokens[nr_token].type=TK_NUM_8;
					// 	for (int j=0;j<substr_len;j++)
          //     tokens[nr_token].str[j]=substr_start[j];
					// 	nr_token++;
					// 	break;
					// case TK_NUM_16:
          //   tokens[nr_token].type=TK_NUM_16;
					// 	for (int j=0;j<substr_len;j++)
          //     tokens[nr_token].str[j]=substr_start[j];
					// 	nr_token++;
					// 	break;
					// case TK_REG:
          //   tokens[nr_token].type=TK_REG;
					// 	for (int j=0;j<substr_len;j++)
          //     tokens[nr_token].str[j]=substr_start[j];
					// 	nr_token++;
					// 	break;	
					// case TK_NUM_10:
          //   tokens[nr_token].type=TK_NUM_10;
					// 	for(int j=0;j<substr_len;j++)
					// 		tokens[nr_token].str[j]=substr_start[j];
					// 	nr_token++;
					// 	break;
					
          default: TODO();
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
	// printf("num:%d\n",nr_token);
  return true;
}

bool check_parentheses(int p,int q){
  if(tokens[p].type=='('&&tokens[q].type==')'){
    int i,j,rValue,flag;
    for(rValue=q-1,i=p+1;i<=rValue;i++){
      if(tokens[i].type=='('){
        for(flag=0,j=rValue;j>i;j--){
          if(tokens[j].type==')'){
            rValue=j-1;
            flag=1;
          }
        }
        if(flag==0) //左括号个数多于右括号的情况
          return false;
      }else if(tokens[i].type==')'){//右括号个数多于左括号的情况
          return false;
      }
    }
    return true;
  }
  return false;
}

int searchDominantOperator(int p,int q){
	int op=-1,cnt=0,op_type=-1;
	for(int i=p;i<=q;i++){
		if(tokens[i].type=='(') cnt++;
		else if(tokens[i].type==')') cnt--;
		//出现在括号中的 and 非运算符
		else if(cnt!=0||tokens[i].type==TK_NUM_16||tokens[i].type==TK_NUM_8
					  ||tokens[i].type==TK_NUM_10) continue;
		//这边暂且不处理单目运算符
		else if(cnt==0){
			
			if(tokens[i].type=='*'||tokens[i].type=='/'||tokens[i].type=='%'){
				if(op_type!='+'&&op_type!='-'&&op_type!=LMOVE&&op_type!=RMOVE&&op_type!=BE&&
				op_type!=SE&&op_type!='>'&&op_type!='<'&&op_type!=TK_FEQ&&op_type!=TK_EQ&&
				op_type!='|'&&op_type!='&'){
					 op=i;op_type=tokens[i].type;
				}
			}
			else if(tokens[i].type=='+'||tokens[i].type=='-'){
				if(op_type!=LMOVE&&op_type!=RMOVE&&op_type!=BE&&
				op_type!=SE&&op_type!='>'&&op_type!='<'&&op_type!=TK_FEQ&&op_type!=TK_EQ&&
				op_type!='|'&&op_type!='&'){
					 op=i;op_type=tokens[i].type;
				}
			}
			else if(tokens[i].type==LMOVE||tokens[i].type==RMOVE){
				if(op_type!=BE&&
				op_type!=SE&&op_type!='>'&&op_type!='<'&&op_type!=TK_FEQ&&op_type!=TK_EQ&&
				op_type!='|'&&op_type!='&'){
					op=i;op_type=tokens[i].type;
				}
			}
			else if(tokens[i].type==BE||tokens[i].type==SE||tokens[i].type=='>'||tokens[i].type=='<'){
				if(op_type!=TK_FEQ&&op_type!=TK_EQ&&op_type!='|'&&op_type!='&'){
					op=i;op_type=tokens[i].type;
				}
			}
			else if(tokens[i].type==TK_FEQ||tokens[i].type==TK_EQ){
				if(op_type!='|'&&op_type!='&'){
					op=i;op_type=tokens[i].type;
				}
			}
			else if(tokens[i].type=='|'||tokens[i].type=='&'){
				op=i;op_type=tokens[i].type;
			}		 
		}
	}
	return op;
}

int eval(int p,int q){
  if(p>q){
    printf("Bad expression_1!\n");
    assert(0);
  }
	else if(p==q){
	int sum=-999;
    if(tokens[p].type==TK_NUM_10){
        sscanf(tokens[p].str,"%d",&sum);	
    }else if(tokens[p].type==TK_NUM_16){
		sscanf(tokens[p].str,"%x",&sum);		
	}else if(tokens[p].type==TK_NUM_8){
		sscanf(tokens[p].str,"%o",&sum);		
	}
	return sum;
  }
	else if(check_parentheses(p,q)==true){
    return eval(p+1,q-1);
  }
	else{
		int op,val_1,val_2,result;
		op=searchDominantOperator(p,q);
		printf("op:%d\n",op);
		if(op==-1){//函数中里面没有判别的 同时又至少有2位
			if (tokens[p].type==TK_NAG){
				sscanf(tokens[p+1].str, "%x", &result);
				return -1*eval(p+1,q);
			}
      		else if(tokens[p].type==DEREF){
				int result;	
				sscanf(tokens[p+1].str,"%x",&result);
				return vaddr_read(result,4);
			}	
			else if(tokens[p].type=='$'){
				for(int i= 0;i<8;i++) {
                    if(strcmp(regsl[i],tokens[p+1].str)==0){
                        return cpu.gpr[i]._32;
                    }
                }
			}
      	}
		val_1=eval(p,op-1);
		printf("val_1:%d\n",val_1);
		val_2=eval(op+1,q);
		printf("val_2:%d\n",val_2);
		switch(op){
			case '+' : 
				return val_1+val_2;
      		case '-' : 
				return val_1-val_2;
      		case '*' : 
				return val_1*val_2;
      		case '/' : 
				return val_1/val_2;
			case LMOVE:
                return  val_1 <<  val_2;
            case RMOVE:
                return  val_1 >>  val_2;
			case SE:
                if ( val_1 <=  val_2) return 1;
                return 0;
            case BE:
                if ( val_1 >=  val_2) return 1;
                return 0;
      		case TK_EQ : 
				return val_1==val_2;
      		case TK_FEQ : 
				return val_1!=val_2;
			case '&' : 
				return val_1&&val_2;
			case '|' : 
				return val_1||val_2;
			case '<' : 
				if(val_1<val_2) return 0;
				return 1;
			case '>' : 
				if(val_1>val_2) return 0;
				return 1;

      		default:assert(0);
		}
  	}
}

bool judge(int x){
	if(tokens[x].type!='+'||tokens[x].type!='-'||tokens[x].type!='*'
	||tokens[x].type!='/'||tokens[x].type!='('||tokens[x].type!='&'
	||tokens[x].type!='|'||tokens[x].type!=TK_NAG||tokens[x].type!=DEREF)
		return false;
	return true;	
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
	    *success = false;
    	return 0;
    }	
	for(int i=0;i<nr_token;i++){
		if(tokens[i].type=='-'&&(i==0||judge(i-1)==true))
			tokens[i].type=TK_NAG;
		else if(tokens[i].type=='*'&&(i==0||judge(i-1))==true)
			tokens[i].type=DEREF;
	}
	int result;
	result=eval(0,nr_token-1);
	printf("result=%d\n",result);
	return result;
}