#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_x(char *args);
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si","Let the program execute n steps",cmd_si },
        { "info","Print the register status and the watchpoint information",cmd_info },
        { "x","Calculate the value of the expression and display the content of the address",cmd_x},
        { "p","Calculate the expression",cmd_p},
	/* TODO: Add more commands */

};

static int cmd_si(char *args)
        { 
          int steps; 
          if (args==NULL)
           {steps=1;
           }
          else {
            steps=atoi(strtok(args," "));
           }
         cpu_exec(steps);
         return 0;
        }

static int cmd_info(char * args)
       {
        if (args==NULL)
         printf("Please input the info r or info w\n");
        else 
        {
          if (strcmp(args, "r")==0)
           {printf("eax: 0x%-10x  %-10d\n",cpu.eax, cpu.eax);
            printf("edx: 0x%-10x  %-10d\n", cpu.edx,cpu.edx);
            printf("ecx: 0x%-10x  %-10d\n", cpu.ecx,cpu.ecx);
            printf("ebx: 0x%-10x  %-10d\n",cpu.ebx,cpu.ebx);
            printf("ebp: 0x%-10x  %-10d\n",cpu.ebp,cpu.ebp);
            printf("esi: 0x%-10x  %-10d\n",cpu.esi,cpu.esi);
            printf("edi: 0x%-10x  %-10d\n",cpu.edi ,cpu.edi);
            printf("esp: 0x%-10x  %-10d\n",cpu.esp,cpu.esp);
           }
         else 
           printf("The info command need a parameter 'r'or'w'\n");
       } 
       return 0;
      }

static int cmd_x(char* args)
      {
      if (args==NULL)
         printf("Please input invalid command\n");
      else 
       {  
          char *arg1=strtok(NULL," ");
          char *arg2=strtok(NULL," ");
          int len,i;
          int addr;

          sscanf(arg1,"%d",&len);
          sscanf(arg2,"%x",&addr);

          printf("0x%x:",addr);
          for (i=0;i<len;i++)
         {
             printf("%x",lnaddr_read(addr,4));
             addr+=4; 
             printf("\n");
         }
       }
         return 0;
      }
            
static int cmd_p(char *args)
      {
        if (args==NULL)
           printf("Input the expression.\n");
        else 
       {
           int num;
           bool success=true;
           num =expr(args,&success);
           if (success)
             printf("result=%d\n",num);
           else 
             printf("Invalid expression.\n");
       }
       return 0;
	}
#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
