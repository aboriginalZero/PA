#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ=255,TK_FEQ=254,TK_LMOVE=253,TK_RMOVE=252,
  TK_NUM_16=251,TK_NUM_10=250,TK_LESS_AND_EQUAL=249,TK_GREATER_AND_EQUAL=248,
  TK_LESS=247,TK_GREATER=246
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
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"!=",TK_FEQ},
  {"-",'-'},
  {"\\*",'*'},
  {"/",'/'},
  // {"<<",TK_LMOVE},
  // {">>",TK_RMOVE},
  // {"0x[0-9a-fA-F]{1,8}",TK_NUM_16},
  {"[1-9]{1,10}",TK_NUM_10},
  // {"<=",TK_LESS_AND_EQUAL},
  // {">=",TK_GREATER_AND_EQUAL},
  // {"<",TK_LESS},
  // {">",TK_GREATER}
  {"\\(",'('},
  {"\\)",')'},
  // {"=",'='},
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
          case '+':
            tokens[nr_token++].type='+';break;          
          case TK_EQ:
            tokens[nr_token++].type=TK_EQ;break;
          case TK_FEQ:
            tokens[nr_token++].type=TK_FEQ;break;
          case '-':
            tokens[nr_token++].type='-';break;
					case '*':
            tokens[nr_token++].type='*';break;
					case '/':
            tokens[nr_token++].type='/';break;
					case TK_NUM_10:
            tokens[nr_token++].type=TK_NUM_10;break;
					case '(':
            tokens[nr_token++].type='(';break;
					case ')':
            tokens[nr_token++].type=')';break;
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

typedef struct op{
	int pos;
	int type;	
}Op;

Op searchDominantOperator(int p,int q){
	Op op;
	op.pos=0;
	op.type=0;
	for(int cnt=0,i=p;i<q;i++){
		if(tokens[i].type=='(') cnt++;
		else if(tokens[i].type==')') cnt--;
		else if(cnt!=0||tokens[i].type==TK_NUM_10) continue;//非运算符和出现在一对括号里面的
		else if(tokens[i].type==TK_EQ||tokens[i].type==TK_FEQ){
			if(op.type!='+'&&op.type!='-'&&op.type!='*'&&op.type!='/'){
				op.pos=i;
				op.type=tokens[i].type;
			}
		}
		else if(tokens[i].type=='*'||tokens[i].type=='/'){
			if(op.type!='+'&&op.type!='-'){
				op.pos=i;
				op.type=tokens[i].type;
			}
		}
		else if(tokens[i].type=='+'||tokens[i].type=='-'){
				op.pos=i;
				op.type=tokens[i].type;
		}
	}
	return op;
}

int eval(int p,int q){
  if(p>q){
    printf("Bad expression!\n");
    assert(0);
  }
	else if(p==q){
    if(tokens[p].type==TK_NUM_10){
      int sum;
      sscanf(tokens[p].str,"%d",&sum);	
      return sum;
    }
    printf("Bad expression!\n");
    assert(0);
  }
	else if(check_parentheses(p,q)==true){
    return eval(p+1,q-1);
  }
	else{
		int val_1,val_2;
		Op op;
		op=searchDominantOperator(p,q);
		val_1=eval(p,op.pos-1);
		val_2=eval(op.pos+1,q);
		switch(op.type){
			case '+' : 
				return val_1+val_2;
      case '-' : 
				return val_1-val_2;
      case '*' : 
				return val_1*val_2;
      case '/' : 
				return val_1/val_2;
      case TK_EQ : 
				return val_1==val_2;
      case TK_FEQ : 
				return val_1!=val_2;
      default:assert(0);
		}
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}