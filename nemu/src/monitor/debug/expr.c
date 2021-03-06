#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define max_string_long 32
#define max_token_num 32
enum {
	NOTYPE = 256, EQ ,NEQ,PLUS, MULTI ,DIVID,AND,OR,LBRACKET,RBRACKET,NUM10,REGISTER,HNUM

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
	{"\\+",'+',4},					// plus
	{"==", EQ,3},						// equal
        {"-",'-',4},
        {"\\*",'*',5},
        {"\\/",'/',5},
        {"\\(",'(',7},
        {"\\)",')',7},
        {"^[1-9][0-9]*|0$",NUM10,0},
	{"\\b0[xX][0-9a-fA-F]+\\b",HNUM,0},
	{"\\$[a-zA-Z]+",REGISTER,0},
	{"!=",NEQ,3},
	{"!",'!',6},
	{"&&",AND,2},
	{"\\|\\|",OR,1},
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
                                char *tmp=e+position+1;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
                                        case NOTYPE:
                                             break;
					case REGISTER:
						tokens[nr_token].type=rules[i].token_type;
						tokens[nr_token].priority=rules[i].priority;
						strncpy(tokens[nr_token].str,tmp,substr_len-1);
						tokens[nr_token].str[substr_len-1]='\0';
						nr_token++;
						break;
					case '+':
					case '-':
					case '*':
					case '/':
					case NUM10:
                       			default:
                                             tokens[nr_token].type=rules[i].token_type;
					     tokens[nr_token].priority=rules[i].priority;
                                             strncpy(tokens[nr_token].str,substr_start,substr_len);
                                             tokens[nr_token].str[substr_len]='\0';
					     nr_token++;
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
         int i;   
         if (tokens[p].type=='('&&tokens[q].type==')')
         {
	       	int lc=0,rc=0;
        	 for (i=p+1;i<q;i++)
        	 {
        	   if (tokens[i].type=='(')
            		 lc++;
           	   if (tokens[i].type==')')
           		 rc++;
          	   if (rc>lc)return false;
	       	}
       	   if (lc==rc)
    	  	 return true;
	}
        return false;
        }

int dominant_operator(int p,int q)
       {
        int i, j,oper=p;
	int min_priority=10;
        for (i=p;i<=q;i++)
         {
		if (tokens[i].type==NUM10||tokens[i].type==HNUM||tokens[i].type==REGISTER)
			continue;
		int cnt=0;
		bool key=true;
		for (j=i-1;j>=p;j--)
		{
          		 if (tokens[i].type=='('&&!cnt)
                	 {
				key=false;break;
			}
			if (tokens[i].type=='(')
				cnt--;
		        if (tokens[i].type==')')
				cnt++;
		}
		if (!key) continue;
				
		if (tokens[i].priority<=min_priority)
		{
			min_priority=tokens[i].priority;
			oper=i;
		}
	}
	return oper;
	}
uint32_t  eval(int p,int q)
{
	if (p>q)
	{
		Assert(p>q,"something happened!\n");
		return 0;
	}
        if (p==q)
	{
		uint32_t num=0;
		if (tokens[p].type==NUM10)
			sscanf(tokens[p].str,"%d",&num);
		if (tokens[p].type==HNUM)
			sscanf(tokens[p].str,"%x",&num);
		if (tokens[p].type==REGISTER)
		{
			if (strlen(tokens[p].str)==3)
			{
				int i;
				for (i=R_EAX;i<=R_EDI;i++)
					if (strcmp(tokens[p].str,regsl[i])==0)break;
					if (i>R_EDI)
					if (strcmp(tokens[p].str,"eip")==0)
						num=cpu.eip;
					else Assert(1,"no this register!\n");
				else num=reg_l(i);
			}
			else if (strlen(tokens[p].str)==2)
			{
				if (tokens[p].str[1]=='x'||tokens[p].str[1]=='p'||tokens[p].str[1]=='i')
				{
					int i;
					for (i=R_AX;i<=R_DI;i++)
						if (strcmp(tokens[p].str,regsw[i])==0)break;
					num=reg_w(i);
				}
				else if (tokens[p].str[1]=='l'||tokens[p].str[1]=='h')
				{
					int i;
					for (i=R_AL;i<=R_BH;i++)
					if (strcmp(tokens[p].str,regsb[i])==0)break;
					num=reg_b(i);
				}
				else assert(1);
			}
		}
		return num;
	}
	else if (check_parentheses(p,q)==true)
	{
		return eval(p+1,q-1);
	}
	else
	{
		int op=dominant_operator(p,q);

	 	uint32_t val1=eval(p,op-1);
		uint32_t val2=eval(op+1,q);

		switch(tokens[op].type)
		{
			case '+': return val1+val2;
			case '-': return val1-val2;
			case '*': return val1*val2;
			case '/': return val1/val2;
			case EQ: return val1==val2;
			case NEQ: return val1!=val2;
			case AND: return val1&&val2;
			case OR: return val1||val2;
			default:break;
		}
	}
	assert(1);
	return -123456;
}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	} 

	/* TODO: Insert codes to evaluate the expression. */
	*success=true;
	return eval(0,nr_token-1);
}

