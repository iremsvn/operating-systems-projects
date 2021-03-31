#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


typedef struct Node
{
    int data;
    struct Node *next;
}Node;


Node* createLinkedList(int n){
  Node* head = 0;
  Node* newptr = 0;
  Node* ptr = 0;

  for(int i = 0; i < n; i++)
  {
    newptr = (Node*)malloc(sizeof(Node));
    newptr->data = 0; //initially elements will be 0
    newptr->next = 0;

    if(head == 0){
        head = newptr;
    }
    else{
        ptr = head;
        while(ptr->next != 0){
            ptr = ptr->next;
        }
        ptr->next = newptr;
    }
  }

  return head;
}

void display(Node* headptr){

    Node* ptr = headptr;

    while(ptr != 0){
        printf("%d\t", ptr->data);
        ptr = ptr->next;
    }

}

void putRandomNumbers(Node* headptr){
    Node* ptr = headptr;

    while(ptr != 0){
        ptr->data = rand() % 10000;
        ptr = ptr->next;
    }
}



int main()
{
    time_t t; //for random function
    srand((unsigned) time(&t));

    printf("Hello world!\n");
    Node* head = 0;
    head = createLinkedList(1000);

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    putRandomNumbers(head);

    struct timeval done_time;
    gettimeofday(&done_time, NULL);

    display(head);

    printf("\n*******************\n");

    printf("seconds : %ld\nmicro seconds : %ld", done_time.tv_sec - current_time.tv_sec, done_time.tv_usec - current_time.tv_usec);

    printf("\n");

    return 0;
}
