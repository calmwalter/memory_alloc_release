#include <stdio.h>
#include <stdlib.h>

typedef struct space
{
  int id;
  int start_point;
  int length;
  int state;
  struct space *next;
  struct space *prev;
} space;

typedef struct fst_node
{
  space *spc;
  struct fst_node *next;
  struct fst_node *prev;
} fst_node;

typedef struct free_space_table
{
  fst_node *head;
} free_space_table;

typedef struct memory
{
  space *head;
} memory;

typedef struct node
{
  int id;
  int length;
} node;

space *find_space(memory *mem, int id)
{
  space *spc = mem->head;
  while (spc)
  {
    if (spc->id == id && spc->state == 1)
    {
      return spc;
    }
    spc = spc->next;
  }
  return NULL;
}

fst_node *find_best_fit_free_space(free_space_table *fst, int length)
{
  fst_node *fn = fst->head;
  while (fn)
  {
    if (fn->spc->length > length)
    {
      return fn;
    }
    fn = fn->next;
  }
  return NULL;
}

void best_fit_recover(free_space_table *fst, space *it)
{
  if(!it){
    printf("TASK ID NOT FOUND.");
    return;
  }
  it->state = 0;
  if (it->prev)
  {
    // if null in previous node: MERGE
    if (!it->prev->state)
    {
      //expand the previous length
      space *prev = it->prev;
      prev->length += it->length;
      prev->next = it->next;
      if (it->next)
      {
        it->next->prev = it->prev;
      }
      //delete current length
      free(it);
      //set previous pointer as the current pointer
      it = prev;
      //remove corresponding free space table node
      fst_node* fn = fst->head;
      while(fn){
        if(fn->spc->id==it->id){
          break;
        }
      }
      if(fn->prev){
        fn->prev->next = fn->next;
      }
      else{
        fst->head = fn->next;
      }
      if(fn->next){
        fn->next->prev = fn->prev;
      }
      free(fn);
    }
  }
  if (it->next)
  {
    // if null in previous node: MERGE
    if (!it->next->state)
    {
      //expand the current length
      space *next = it->next;
      it->length += next->length;
      it->next = next->next;
      if (next->next)
      {
        next->next->prev = it;
      }

      //remove corresponding free space table node
      fst_node* fn = fst->head;
      while(fn){
        if(fn->spc->id==next->id){
          break;
        }
      }
      if(fn->prev){
        fn->prev->next = fn->next;
      }
      else{
        fst->head = fn->next;
      }
      if(fn->next){
        fn->next->prev = fn->prev;
      }
      free(fn);
      //delete next length
      free(next);
    }
  }
  //add new free space table node
  fst_node* fn = (fst_node*)malloc(sizeof(struct fst_node));
  fn->spc = it;
  fst_node* tmp = fst->head;
  if (!tmp)
  {
    fst->head = fn;
    fn->next = NULL;
    fn->prev = NULL;
    it->id = -1;
    return ;
  }
  if(tmp->spc->length>it->length){
    fn->next = fst->head;
    fn->prev = NULL;
    if(fst->head){
      fst->head->prev = fn;
    }
    fst->head = fn;
    it->id = -1;
    return ;
  }

  while(tmp->next){
    if(tmp->next->spc->length>it->length){
      break;
    }
    tmp = tmp->next;
  }
  fn->next = tmp->next;
  fn->prev = tmp;
  if(tmp->next){
    tmp->next->prev = fn;
  }
  tmp->next = fn;
  it->id = -1;
}

void best_fit_allocation(memory* mem,free_space_table *fst, node *n)
{
  if (!n)
  {
    return ;
  }
  space* spc_tmp = mem->head;
  while(spc_tmp){
    if (spc_tmp->id==n->id)
    {
      printf("TASK ID ALREADY EXISTS.\n");
      return ;
    }
    spc_tmp = spc_tmp->next;
  }
  fst_node *fn = find_best_fit_free_space(fst, n->length);
  space *fre_spc = fn->spc;
  if (fre_spc)
  {
    fre_spc->id = n->id;
    fre_spc->state = 1;
    space *spc=NULL;
    if (n->length < fre_spc->length)
    {
      // allocate new space and seperate current space
      spc = (space *)malloc(sizeof(struct space));
      spc->id = -1;
      spc->length = fre_spc->length - n->length;
      spc->start_point = fre_spc->start_point + n->length;
      spc->state = 0;
      spc->next = fre_spc->next;
      spc->prev = fre_spc;
      fre_spc->length = n->length;
      fre_spc->next = spc;
    }
    //remove it in the free space table
    fst_node* fn_prev = fn->prev;
    if (fn->prev)
    {
      fn->prev->next = fn->next;
    }
    else{
      fst->head = fn->next;
    }
    if (fn->next)
    {
      fn->next->prev = fn->prev;
    }
    free(fn);
    //adjust free space position
    if(spc){
      while (fn_prev)
      {
        if(fn_prev->spc->length<spc->length){
          break;
        }
        fn_prev = fn_prev->prev;
      }
      fst_node* new_fn = (fst_node*)malloc(sizeof(struct fst_node));
      new_fn->spc = spc;
      if (fn_prev)
      {
        new_fn->next = fn_prev->next;
        if (fn_prev->next)
        {
          fn_prev->next->prev = new_fn;
        }
        new_fn->prev = fn_prev;
        fn_prev->next = new_fn;
      }
      else
      {
        new_fn->next = fst->head;
        if(fst->head)
        {
          fst->head->prev = new_fn;
        }
        fst->head = new_fn;
        new_fn->prev = NULL;
      }
    }
  }
  else
  {
    printf("NO ENOUGH SPACE.\n");
  }
  return ;
}
fst_node *find_fst_postion(free_space_table *fst, int length)
{
  fst_node *fn = fst->head;
  if (!fn)
  {
    return NULL;
  }
  if (fn->spc->length > length)
  {
    return NULL;
  }
  while (fn->next)
  {
    if (fn->next->spc->length > length)
    {
      return fn;
    }
    fn = fn->next;
  }
  return fn;
}
void add_fst_node(free_space_table *fst, fst_node *fn)
{
  fst_node *pos = find_fst_postion(fst, fn->spc->length);
  if (!pos)
  {
    fn->prev = NULL;
    fn->next = fst->head;
    if (fst->head)
    {
      fst->head->prev = fn;
    }
    fst->head = fn;
  }
  else
  {
    fn->next = pos->next;
    fn->prev = pos;
    if (pos->next)
    {
      pos->next->prev = fn;
    }
    pos->next = fn;
  }
}

void print_free_space_table(free_space_table *fst)
{
  fst_node *fn = fst->head;
  printf("\nFREE SPACE TABLE\n");
  printf("+");
  for (int i = 0; i < 31; i++)
  {
    printf("-");
  }
  printf("+\n");
  printf("|%-15s|%-15s|\n","start point", "length");
  printf("+");
  for (int i = 0; i < 31; i++)
  {
    printf("-");
  }
  printf("+\n");
  while (fn)
  {
    printf("|%-15d|%-15d|\n", fn->spc->start_point, fn->spc->length);
    fn = fn->next;
  }
  printf("+");
  for (int i = 0; i < 31; i++)
  {
    printf("-");
  }
  printf("+\n");
}

void print_memory(memory *mem)
{
  space *spc = mem->head;
  printf("\nMEMORY TABLE\n");
  printf("+");
  for (int i = 0; i < 63; i++)
  {
    printf("-");
  }
  printf("+\n");
  printf("|%-15s|%-15s|%-15s|%-15s|\n", "task id", "start point", "length", "state");
  printf("+");
  for (int i = 0; i < 63; i++)
  {
    printf("-");
  }
  printf("+\n");
  while (spc)
  {
    printf("|%-15d|%-15d|%-15d|%-15d|\n", spc->id, spc->start_point, spc->length, spc->state);
    spc = spc->next;
  }
  printf("+");
  for (int i = 0; i < 63; i++)
  {
    printf("-");
  }
  printf("+\n");
}

int main()
{
  //initialize
  memory mem;
  mem.head = NULL;
  free_space_table fst;
  fst.head = NULL;
  int base[6] = {0, 10, 20, 45, 65, 110};
  int base_length[6] = {10, 10, 25, 20, 45, 146};
  int state[6] = {1, 1, 1, 0, 1, 0};
  int id[6] = {0, 1, 4, -1, 2, -1};
  for (int i = 5; i >= 0; i--)
  {
    space *it = (space *)malloc(sizeof(struct space));
    it->prev = NULL;
    it->next = mem.head;
    it->start_point = base[i];
    it->id = id[i];
    it->length = base_length[i];
    it->state = state[i];
    if (mem.head)
    {
      mem.head->prev = it;
    }
    mem.head = it;
    if (!it->state)
    {
      fst_node *fn = (fst_node *)malloc(sizeof(struct fst_node));
      fn->spc = it;
      add_fst_node(&fst, fn);
    }
  }
  while (1)
  {
    print_free_space_table(&fst);
    print_memory(&mem);
    printf("+----------------------+\n");
    printf("|0. allocation task    |\n");
    printf("|1. recover task       |\n");
    printf("|2. quit               |\n");
    printf("+----------------------+\n");

    int select;
    scanf("%d", &select);
    if (select == 0)
    {
      node new_node;
      printf("enter the id of the task:");
      scanf("%d", &new_node.id);
      printf("enter the size of the task:");
      scanf("%d", &new_node.length);
      best_fit_allocation(&mem, &fst, &new_node);
    }
    else if (select == 1)
    {
      space *spc;
      int id;
      printf("enter the task id:");
      scanf("%d", &id);
      spc = find_space(&mem, id);
      if (!spc)
      {
        printf("NO SUCH TASK IN MEMORY.\n");
      }
      else
      {
        best_fit_recover(&fst, spc);
      }
    }
    else if (select == 2)
    {
      break;
    }
    else
    {
      printf("INPUT ERROR.\n");
    }
  }
  return 0;
}
