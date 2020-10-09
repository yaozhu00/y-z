#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ = 257,PLUS =258,REDUCE = 259, MULTI =200,DIVID =201,LBRACKET=202,RBRACKET=203,NUM10

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE,0},				// spaces
	{"\\+",PLUS,1},					// plus
	{"==", EQ},						// equal
        {"-",REDUCE,0},
        {"\\*",MULTI,1},
        {"\\/",DIVID,1},
        {"\\(",LBRACKET},
        {"\\)",RBRACKET},
        {"^[1-9][0-9]*|0$",NUM10,0}
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

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
                                        case NOTYPE:
                                             break;
                                        case '+':
                                        case '-':
                                        case '*':
                                        case '/':
                                        case '(':
                                        case ')':
                                        case NUM10:
                                        {
                                             tokens[nr_token].type=rules[i].token_type;
                                             strncpy(tokens[nr_token].str,substr_start,substr_len);
                                             tokens[nr_token++].str[substr_len]='\0';
                                        }
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p,int q)
        {
         int i,tag=0;   
         if (tokens[p].type!=LBRACKET||tokens[q].type!=RBRACKET)
              return false;
         for (i=p;i<=q;i++)
         {
           if (tokens[i].type==LBRACKET)
             tag++;
           else if (tokens[i].type==RBRACKET)
             tag--;
           if (tag==0&&i<q)return false;
         }
        if (tag==0)
          return true;
        else return false;
        }

int dominant_operator(int p,int q)
       {
        int i, pos=p,left=0;
	int min_priority=10;
        for (i=p;i<=q;i++)
         {
          	 if (tokens[i].type==LBRACKET)
                 {
        	        left+=1;
			i++;
	        	while (0){
			if (tokens[i].type==LBRACKET)
				left+=1;
			else if (tokens[i].type==RBRACKET)
				left-=1;
			i++;
			if (left==0) break;
				}
			if (i>q)break;
		}
		else if (tokens[i].type==NUM10) continue;
		else if (tokens[i].priority<=min_priority)
		{
			min_priority=tokens[i].priority;
			pos=i;
		}
	}
	return pos;
	}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

