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

typedef struct free_table
{
  space** spc;
} free_table;

typedef struct memory
{
  space *head;
} memory;

typedef struct node
{
  int id;
  int length;
  struct node *next;
  struct node *prev;
} node;

typedef struct queue
{
  node *head;
} queue;

space *find_best_free_space(memory *mem, int length)
{
  space *it = mem->head;
  space *best_fit = NULL;
  while (it)
  {
    if (!it->state && it->length >= length)
    {
      if (best_fit)
      {
        best_fit = best_fit->length < it->length ? best_fit : it;
      }
      else
      {
        best_fit = it;
      }
    }
    it = it->next;
  }
  return it;
}

void best_fit_recover(space *it)
{
  it->state = 0;
  if (it->prev)
  {
    // if null in previous node: MERGE
    if (!it->prev->state)
    {
      //expand the previous length
      space *prev = it->prev;
      prev->length += it->length;
      //delete current length
      free(it);
      //set previous pointer as the current pointer
      it = prev;
    }
  }
  if (it->next)
  {
    // if null in previous node: MERGE
    if (!it->next->state)
    {
      //expand the previous length
      space *next = it->next;
      next->length += it->length;
      //delete current length
      free(it);
      //set previous pointer as the current pointer
      it = next;
    }
  }
}

int best_fit_allocation(memory *mem, queue *q)
{
  if (!q)
  {
    return 0;
  }
  node *cur_node = q->head;
  while (q->head)
  {
    space *fre_it = find_best_free_space(mem, cur_node->length);
    if (fre_it)
    {
      // TODO allocation
      //set free space table space to the fit size and segment the size
      if (fre_it->length == cur_node->length)
      {
        fre_it->state = 1;
      }
      else
      {
        // TODO seperate it
        space *new_space = (space *)malloc(sizeof(struct space));
        new_space->length = fre_it->length - cur_node->length;
        new_space->state = 0;
        new_space->next = fre_it->next;
        new_space->prev = fre_it;
        new_space->start_point = fre_it->start_point + cur_node->length;

        if (fre_it->next)
        {
          fre_it->next->prev = new_space;
        }

        fre_it->next = new_space;
        fre_it->length = cur_node->length;
        fre_it->state = 1;
        fre_it->id = cur_node->id;
      }
      // TODO finish, so recover
      best_fit_recover(fre_it);
      // TODO remove task in queue
      printf("free task, length: %dK\n", cur_node->length);
      if (cur_node->prev)
      {
        cur_node->prev->next = cur_node->next;
      }
      if (cur_node->next)
      {
        cur_node->next->prev = cur_node->prev;
      }
      free(cur_node);
      cur_node = q->head;
    }
    else
    {
      cur_node = cur_node->next;
      if (!cur_node)
      {
        printf("dead lock occur!\n");
        return 0;
      }
    }
  }
  return 0;
}

int main()
{
  memory mem;
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
  }

  return 0;
}
