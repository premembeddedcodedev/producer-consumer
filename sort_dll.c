#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { N = 40 };

typedef struct Person
{
	int  code;
	char name[N];
	char job[N];
	int  salary;
} Person;

typedef struct Node 
{
	Person person;  
	struct Node *prev;
	struct Node *next;
} Node;

typedef struct List
{
	Node *next;
	Node *prev;
} List;

//struct list_head {
struct list_head {


}

int push_back( List *list, const Person *person )
{
	Node *new_node = malloc( sizeof( Node ) );
	int success = new_node != NULL;



	if ( success )
	{
		new_node->person = *person;
		new_node->prev   = list->prev;
		new_node->next   = NULL;

		if ( list->prev == NULL )
		{
			list->next = list->prev = new_node; 
		}
		else
		{
			list->prev = list->prev->next = new_node;
		}
	}

	return success;
}

void display( const List *list )
{
	for ( Node *current = list->next; current != NULL; current = current->next )
	{
		printf( "code = %d, name = %s, job = %s, salary = %d\n",
				current->person.code, current->person.name, 
				current->person.job, current->person.salary );
	}
}

void swap( Node **current )  
{  
	Node *tmp = *current;  

	*current = ( *current )->next;  

	tmp->next = ( *current )->next;
	( *current )->next = tmp;

	( *current )->prev = tmp->prev;
	tmp->prev = *current;

	if ( tmp->next != NULL )
	{
		tmp->next->prev = tmp;
	}
}

void clear( List *list )
{
	while ( list->next != NULL )
	{
		Node *current = list->next;
		list->next = list->next->next;
		free( current );
	}

	list->prev = list->next;
}

void sort( List *list, int cmp( const void *, const void * ) )
{
	if ( list->next != NULL )
	{
		int first_iteration = 1;

		for ( Node **first = &list->next, *sorted = NULL, *last = NULL;
				( *first )->next != last;
				last = sorted )
		{
			Node **current = first;
			sorted = ( *first )->next;

			for ( ; ( *current )->next != last; current = &( *current )->next )
			{
				if ( cmp( &( *current )->person, &( *current )->next->person ) > 0 )
				{
					swap( current );
					sorted = ( *current )->next;
				}
			}

			if ( first_iteration )
			{
				list->prev = *current;
				first_iteration = 0;
			}               
		}             
	}         
}

int cmp_by_salary( const void *a, const void *b )
{
	const Person *left  = a;
	const Person *right = b;

	return ( right->salary < left->salary ) - ( left->salary < right->salary );
}

int cmp_by_code( const void *a, const void *b )
{
	const Person *left  = a;
	const Person *right = b;

	return ( right->code < left->code ) - ( left->code < right->code );
}

int cmp_by_name( const void *a, const void *b )
{
	const Person *left  = a;
	const Person *right = b;

	return strcmp( left->name, right->name );
}

int main(void) 
{
	List list = { .next = NULL, .prev = NULL };

	Person person[] =
	{
		{ .code = 1, .name = "RaCo", .job = "programmer", .salary = 1000 },
		{ .code = 2, .name = "Another RaCo", .job = "programmer", .salary = 500 },
		{ .code = 3, .name = "One more RaCo", .job = "programmer", .salary = 1500 },
	};
	const size_t n = sizeof( person ) / sizeof( *person );

	puts( "Original list:" );
	for ( size_t i = 0; i < n; i++ )
	{
		push_back( &list, person + i );
	}

	display( &list );
	putchar( '\n' );

	sort( &list, cmp_by_salary );

	puts( "list sorted by salary:" );
	display( &list );
	putchar( '\n' );

	sort( &list, cmp_by_name );

	puts( "list sorted by name:" );
	display( &list );
	putchar( '\n' );

	sort( &list, cmp_by_code );

	puts( "list sorted by code:" );
	display( &list );
	putchar( '\n' );

	printf( "Debug output. The pointer prev points to %s\n", list.prev->person.name );

	clear( &list );

	return 0;
}
