#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string.h>
#include <curses.h>
#include <wctype.h>
#include <sys/time.h>
#include <time.h>


#define CTRL(c) ((c) & 037)



struct doc {
	unsigned char *line;      /* Character in the line */
	unsigned long *ctl;	  /* Controls for this line */
	int line_width;
	int line_nr;
	int header;		  /* Paragraph infos */
	int align;		  /* Aligment of this line */
	struct doc * next;
	struct doc * prev;
	};

#define BOLD 1<<0
#define ITAL 1<<1
#define UNDR 1<<2
#define CENTER 1<<3
#define RIGHT 1<<4

#define HEAD 1<<0
#define END 1<<1
#define PAGE 1<<1

struct doc *first;
struct doc *last;
struct doc *cur;
struct doc *vis;
int cur_y=0;
int cur_x=0;
int max_x;
int max_y;
int ctrl=0;
int parag=0;


struct doc * New_Line(struct doc ** ffirst, struct doc **llast )
{
 struct doc *new;
 struct doc *upd;
 new=(struct doc *) malloc(sizeof(struct doc));
     if((*ffirst == NULL) && (*llast == NULL)) *ffirst=*llast=new;
			
     else {
	 new->next=(*llast)->next;
	 (*llast)->next=new;
	 new->prev=*llast;
	 if(new->next != NULL) new->next->prev=new;
	 if (*llast == NULL) *llast=new;
//	 if(*ffirst == NULL) *ffirst=new;
      }
     
 new->line_nr= (new->prev != 0) ? new->prev->line_nr +1 :1 ;
 new->line=(unsigned char *) 0;
 new->ctl=(unsigned long *) 0;

 
 for(upd=new->next;upd!=0;upd=upd->next)
	upd->line_nr=(new->prev !=0) ? upd->prev->line_nr +1 :1;

 return new;
} 


void Del_line( struct doc *this,  struct doc ** ffirst, struct doc **llast)
{
 struct doc *upd;
 
 if (this->prev !=0 )this->prev->next=this->next;
	else (*ffirst)=this->next;
 if (this->next !=0) this->next->prev=this->prev;
	else (*llast)=this->prev;
 if( (*ffirst) == this) (*ffirst)=this->next;
 if( (*llast) == this) (*llast)=this->prev;

 if(this->line != 0) free(this->line);
 if(this->ctl != 0) free(this->ctl);

 for(upd=this->next;upd!=0;upd=upd->next)
	upd->line_nr=(this->prev !=0) ? upd->prev->line_nr +1 :1;
 free(this);

}

Add_Char(struct doc * this, char ch, char ctrl, int x)
{
 int len=strlen(this->line);
 int to_move;
 if (len < x) to_move=len; else to_move=len-x;
 
  if (to_move > 0) {
		memmove(&this->line[x+1],&this->line[x],to_move);
		memmove(&this->ctl[x+1],&this->ctl[x],to_move*sizeof(long));
		}	
 if (len < x) to_move=len; else to_move=x;
	
 this->line[to_move]=ch;
 this->ctl[to_move]=ctrl;

}


Del_Char(struct doc * this, int x)
{
 int len=strlen(this->line);
 int to_move;
 if (len < x) to_move=len; else to_move=len-x;
 if(to_move>0) 
  {
    memmove(&this->line[x-1],&this->line[x],to_move);
    memmove(&this->ctl[x-1],&this->ctl[x],to_move*sizeof(long));
  }
 this->line[len-1]=0;

}

sc_init()
{
  initscr();
        raw();
        noecho();
        meta(NULL, TRUE);
        nonl();
        idlok(stdscr, TRUE);
        idcok(stdscr, TRUE);
        scrollok(stdscr, FALSE);
        intrflush(stdscr, FALSE);
        //notimeout(stdscr, TRUE);
        keypad(stdscr, TRUE);
}
 
sc_header()
{
int x;

 for(x=1;x<max_x-1;x++)
 {
   if ((x%10) ==0) mvprintw(0,(x),"%d",(x/10)%10);
   else
   if ((x%5) ==0 ) mvaddnstr(0,(x),"|",1);  
   else 
    mvaddnstr(0,(x),".",1);  
 }
 
 mvaddnstr(0, 0, "[",1);
 mvaddnstr(0, max_x-1, "]",1);
}

sc_display()
{
struct doc *new;
int y=0;
int l=0;
int x;
int off;
int len;

erase();
sc_header();
for(new=vis;((new!=0) && (l<=max_y-3)) ;new=new->next)
{
 if(new==cur) mvprintw((y+1),0,">");
        else        mvprintw((y+1),0," ");
// mvaddnstr((y++)+1,1,new->line,80);
 if(new->line !=0 )
 if(new->header <4) off=2;  else { off=0; } 
 len=strlen(new->line);
 if(len>max_x) len-=off;
 for(x=0;x<len;x++)
	{
		int attr=A_NORMAL;
		int just=0;
		if ((new->ctl[x] & BOLD) == BOLD)  attr |= A_BOLD;
		if ((new->ctl[x] & UNDR) == UNDR)  attr |= A_UNDERLINE;
		if ((new->ctl[x] & ITAL) == ITAL)  attr |= A_ITALIC;
		if ((new->align & CENTER) == CENTER) just=(max_x-off-2)/2-len/2;	
		if ((new->align & RIGHT) == RIGHT) just=max_x-off-2-len;	
	          if (attr !=0 )
			 attrset(attr);		
		   mvaddch(y+1,x+1+off+just,new->line[x]);
	          if (attr !=0 )
			 attrset(A_NORMAL);		
		
	}
 if (new->header <4) {y++; l++; } else { y+=2; l+=2;}
 }
refresh();


}

main()
{

struct doc *new;
struct doc *new1;
int v=1;
int off=0;
int just=0;
int off_x=0;

sc_init();
getmaxyx(stdscr,max_y,max_x);

new=New_Line(&first,&last);
new->line=malloc(80);
new->ctl=malloc(80*sizeof(long));
new->align=CENTER;
sprintf(new->line, "[HEAD]");

new=New_Line(&first,&last);
new->line=malloc(80);
new->ctl=malloc(80*sizeof(long));
new->align=CENTER;
sprintf(new->line, "[END DOCUMENT]");

new1=New_Line(&first,&last);
new1->line=malloc(80);
new1->ctl=malloc(80*sizeof(long));
sprintf(new1->line, "Roman 2");

new=New_Line(&first,&last);
new->line=malloc(80);
new->ctl=malloc(80*sizeof(long));
sprintf(new->line, "Roman 3");

new=New_Line(&new1->prev,&new1);
new->line=malloc(80);
new->ctl=malloc(80*sizeof(long));
sprintf(new->line, "Roman 4");


cur=last;
vis=first;
sc_display();

//Del_line(new1,&first,&last);


cur=first->next;

while(1)
{
wint_t c;
getmaxyx(stdscr,max_y,max_x);
		int r = getch();
		switch(r) {
		case KEY_UP:	if((cur !=0 ) && (cur->prev !=0) ) {
							cur=cur->prev;
						if(cur->header >=4) v--;
					if(v>0) 
						 v--; 
					 else
						vis=vis->prev;

						}
						break; 
		case KEY_DOWN:  if((cur !=0 ) && (cur->next !=0) ) {
							if (cur->header >= 4) v++;
						       cur=cur->next; 
						    
					if(v>=max_y-3) { 
					cur_y=max_y-3; 
					vis=vis->next;  }
					else  v++; 
					}
					break;

		case KEY_NPAGE:
				if (cur != 0 ){
				int tmpi;
				for(tmpi=max_y-3;((cur->next !=0 ) && (tmpi > 0)); tmpi--)
				{
				cur=cur->next;
				vis=vis->next;
				}
				}
				break;

		case KEY_PPAGE:
				if (cur !=0 ){
                                int tmpi;
                                for(tmpi=max_y-3;((vis->prev !=0 ) && (tmpi > 0)); tmpi--, cur=cur->prev, vis=vis->prev);
				}
                                break;

		case KEY_LEFT: if(cur !=0 ) { (cur_x > 0)? cur_x--:cur_x;  if(ctrl !=0 )cur->ctl[cur_x+1]=ctrl; } break;
		case KEY_RIGHT: if(cur!=0 ) { (cur_x< 80)? cur_x++:cur_x; if(ctrl !=0 )cur->ctl[cur_x-1]=ctrl; } break;
 		case CTRL('D'):  if (cur !=0 ){
					struct doc *tmp=(cur ==first )? cur->next:cur->prev;	
					if (cur==vis) vis =vis->next;
					Del_line(cur,&first,&last); 
					cur	= tmp; v--;  } break;

		case KEY_BACKSPACE:
		case CTRL('G'):  if (cur !=0 ){ if (cur_x >0) Del_Char(cur, cur_x--); } break;
		case KEY_DC:	if (cur !=0 ) { Del_Char(cur, ++cur_x); cur_x--; } break;
		case CTRL('B'):  ctrl ^= BOLD; break;
		case CTRL('U'):  ctrl ^= UNDR; break;
		case CTRL('I'):  ctrl ^= ITAL; break;
		case CTRL('P'):  {int ch; ch=getchar(); if((ch>='0' ) && (ch<='9')) parag=ch-'0'; if(cur!=0) cur->header=parag; } break;
		case CTRL('O'):  {int ch; ch=getchar(); 
					  switch(ch) 
						{
							case 'r': just=RIGHT; break;
							case 'c': just=CENTER; break;
							case 'l': just=0;  break;
						 }
				  if(cur !=0 ) cur->align=just;
				 } break;
					
		case 13:
		case '\n':
		case KEY_ENTER: {
				 if(first == 0 ) 
					{
					new=New_Line(&first,&last);
					vis=new;
					v=-1;
					}
				  else
				  	new=New_Line(&cur->prev,&cur);
				  new->line=realloc(NULL,80);
				  memset(new->line,0,80);
				  new->ctl=realloc(NULL,80*sizeof(long));
				  memset(new->ctl,0,80*sizeof(long));
				  if (cur->header >= 4) v++;
				  cur=new;
				  new->header=parag;
				  new->align=just;
				
				  cur_x=0;
					if(v>=max_y-3)  vis=vis->next; 
					 else  v++; 
				}

				break;

		case KEY_RESIZE:
				break;
 
		
		default: 
				if (cur == 0 ) {
				 if(first == 0 ) 
					new=New_Line(&first,&last);
				  else
				 	new=New_Line(&cur->prev,&cur);
                                  new->line=realloc(NULL,80);
                                  memset(new->line,0,80);
                                  new->ctl=realloc(NULL,80*sizeof(long));
                                  memset(new->ctl,0,80*sizeof(long));
                                  vis =cur=new;
                                  cur_x=0;
			         } 	
				Add_Char(cur,r,ctrl,cur_x);
				cur->align=just;
				cur_x++;
				
				break;
		}
		cur_y=((cur !=0) && (cur->line_nr != 0))? cur->line_nr:1;	
		if (r=='q' ) {
				endwin();
				exit(0);
				}
		/*
		if(cur==0) {
				fprintf(stderr,"something is very wrong");
				endwin();
				exit(0);
				}
		*/
		if (cur !=0 )
			if(cur->header<5) off=2; else off=0; 
		else 
			if(parag<5) off=2; else off=0; 

		sc_display();
		off_x=0;
		if(cur !=0 )
		{
			cur_x=(strlen(cur->line) < cur_x)? strlen(cur->line): cur_x;
			just=cur->align;
			if(just == RIGHT) off_x=max_x-off-2-strlen(cur->line);
			if(just == CENTER) off_x=(max_x-off-2)/2-strlen(cur->line)/2;
		}
		else cur_x=1;
		mvprintw(max_y-1,0,"cursor: %d:%d [%s]    v:%d ctl:%x head: %d  max_y: %d  just: %d X:%d", cur_x, cur_y,keyname(r),v,ctrl,parag,max_y,just, cur_x+off+off_x+1);

		move(v+1,cur_x+off+off_x+1);
}

//for(new=first;new!=0;new=new->next)
//printf("\n %d [%s]",new->line_nr,new->line);

}

