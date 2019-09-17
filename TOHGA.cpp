/**********************************************TOH VIA GENETIC ALGORITHM****************************************/

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <cmath>

#define A_TO_B 1
#define A_TO_C 2
#define B_TO_A 16
#define B_TO_C 18
#define C_TO_A 32
#define C_TO_B 33
#define MAX_OPERATIONS 6 //count of possible moves
#define NUM_OPERATIONS 12 //max operations to put all discs in final peg. -> no. of steps in plan
#define POPULATION_SIZE 200
#define CROSSOVER_PROB 0.07
#define MUTATION_PROB 0.01

#define TRACE_ON 1
#define TRACE_OFF 0

using namespace std;

typedef struct 
{
int disc[3];
int count;
} peg_t; // peg_t represents a peg with disc count as 'count' and disc seqence as disc[0] disc[1] and disc[2]

typedef struct 
{
int op_count;
int plan[NUM_OPERATIONS+1];
double fitness;
} solution_t;//represents a solution with fitness value 'fitness', moves as plan[0...NUM_OPS] and no. of active moves as op_count

int moves[MAX_OPERATIONS]={A_TO_B, A_TO_C, B_TO_A, B_TO_C, C_TO_A, C_TO_B }; //possible moves

peg_t pegs[3];// represents 3 pegs
solution_t solutions[2][POPULATION_SIZE];// represents current and the new population of solutions

double maxval, avgval, minval, sum;

double randomDouble();
int randMax(int r);
int findmin(int a,int b);
void initPopulation(int curr_pop);
void printPopulation(int curr_pop);
void compute_population_fitness( int curr_pop);
double compute_fitness(int curr_pop, int member, int trace);
int select_parent(int curr_pop);
int mutate(int x);
int perform_ga(int curr_pop);
void print_towers();
void trace_best(int curr);


int main()
{
	time_t t;
	srand((unsigned) time(&t));
	int cur = 0;
  	int generation = 0;
	initPopulation(cur);
	print_towers();
	compute_population_fitness(cur);
	
	//printPopulation(cur);
	while ((avgval < (0.999 * maxval)) && (maxval < 75.0) && generation < 20000) 
	{
		cur = perform_ga(cur);
    	compute_population_fitness(cur);
		if (((generation++) % 500) == 0) 
		{
      		printf("%6d: %2g %5g %2g\n", generation, minval, avgval, maxval);
    	}
	}

  	printf("%6d: %2g %5g %2g\n", generation, minval, avgval, maxval);
  	trace_best(cur);
  	print_towers();
	return 0;
}

double randomDouble()
{
		
	return ((rand()%1001)/1000.0);
}

int randMax(int r)// returns a positive random number <= r
{

	return (rand()%r);
}

int findmin(int a, int b)
{

	return ((a<b)?a:b);
}

void initPopulation(int curr_pop)
{
	int i, discs = 3;
	int half = (NUM_OPERATIONS >> 1);
	for (i = 0; i < POPULATION_SIZE; i++)
		{
			for(int j = 0; j<NUM_OPERATIONS; j++)
			{
				solutions[curr_pop][i].plan[j] = randMax(MAX_OPERATIONS);
			}
			solutions[curr_pop][i].op_count = half + randMax(half);
  			solutions[curr_pop][i].fitness = 0.0;
		}
	for (i = 0 ; i < 3 ; i++) 
	{
		pegs[0].disc[i] = discs--;//source peg contains discs 3,2,1 from bottom to top
		pegs[1].disc[i] = 0;
		pegs[2].disc[i] = 0;
	}

	pegs[0].count = 3;// source peg initially has 3 discs
	pegs[1].count = 0;
	pegs[2].count = 0;
}

void printPopulation(int curr_pop)
{

	for (int i = 0; i < POPULATION_SIZE; i++)
		{
			cout<<"Population member "<<i+1<<endl;
			for(int j = 0; j<NUM_OPERATIONS; j++)
			{
				cout<<solutions[curr_pop][i].plan[j]<<"  ";
			}
			cout<<endl;
			cout<<solutions[curr_pop][i].op_count<<endl;
  			cout<<solutions[curr_pop][i].fitness<<endl;  
		}
}

void compute_population_fitness( int curr_pop)
{
	double fitness;
  	sum = 0.0;
  	minval = 999.0;
  	maxval = 0.0;

	for(int i=0; i<POPULATION_SIZE; i++)
	{
		fitness = compute_fitness(curr_pop,i,TRACE_OFF);
		sum += fitness;
		if (fitness > maxval) 
			maxval = fitness;
    	if (fitness < minval) 
    		minval = fitness;
		solutions[curr_pop][i].fitness = fitness;
	}

	avgval = sum / (double)POPULATION_SIZE;
}

double compute_fitness( int curr_pop, int member, int trace)
{
	int i, from, to, move, discs=3, illegal_moves = 0;
	double fitness;

	/* Initialize the pegs */
	for (i = 0 ; i < 3 ; i++) 
	{
		pegs[0].disc[i] = discs--;//source peg contains discs 3,2,1 from bottom to top
		pegs[1].disc[i] = 0;
		pegs[2].disc[i] = 0;
	}

	pegs[0].count = 3;// source peg initially has 3 discs
	pegs[1].count = 0;
	pegs[2].count = 0;

	if (trace) cout<<"Trace : ";

	for (i = 0 ; i < solutions[curr_pop][member].op_count ; i++) // for each active move that has been made
	{
		/* Get the actual move from the moves array */
		move = moves[solutions[curr_pop][member].plan[i]];
		/* Find the source peg */
		from = (move >> 4); // gives the source peg
		if (pegs[from].count == 0) // if source has no disc to move but still we are moving
		{
      		if (trace) 
      			printf("[%02x] ", moves[solutions[curr_pop][member].plan[i]]);
			illegal_moves++;
		} 
		else 
		{
			/* Find the destination peg */	
			to = move & 15;// gives the destination peg
			/* Ensure it’s a legal move--- if destn. has no discs in it or destn. has bigger disc than source disc*/
			if ((pegs[to].count == 0) ||(pegs[from].disc[pegs[from].count-1] < pegs[to].disc[pegs[to].count-1])) 
			{
				if (trace) 
					printf("%02x ", moves[solutions[curr_pop][member].plan[i]]);
				/* Perform the move, update the pegs configuration */
				pegs[from].count--;
				pegs[to].disc[pegs[to].count] = pegs[from].disc[pegs[from].count];
				pegs[from].disc[pegs[from].count] = 0;
				pegs[to].count++;
			} 
			else 
			{
				if (trace) printf("[%02x] ", moves[solutions[curr_pop][member].plan[i]]);
				illegal_moves++;
			}
		}
	}
	/* Calculate the fitness */
	fitness = (double)(pegs[2].count*25) - (double)illegal_moves;
	if (fitness < 0.0) 
		fitness = 0.0;
	if (trace)
		cout<<endl<<endl;
	return fitness;
}

int select_parent( int curr_pop )// Roulette wheel selection
{
	int i = randMax(POPULATION_SIZE);// choose a random member in curr population
	int count = POPULATION_SIZE;
	double select=0.0;
	while (count--) // searching in entire population
	{
		select = solutions[curr_pop][i].fitness; // fitness of randomly chosen member
		if (randomDouble() < (select / sum)) //roulette wheel condition
			return i;//i is the parent
		if (++i >= POPULATION_SIZE) // incrementing i
			i = 0;
	}
	return( randMax(POPULATION_SIZE) );
}

int mutate(int x)
{

	return ((randomDouble() < MUTATION_PROB) ? randMax(MAX_OPERATIONS) : x);
}

int perform_ga( int curr_pop )
{
	int i, j, new_pop, parent_1, parent_2, crossover;

	new_pop = (curr_pop == 0) ? 1 : 0;
	for ( i = 0 ; i < POPULATION_SIZE ; i+=2 ) 
	{
		/* i is child_1, i+1 is child_2 */
		parent_1 = select_parent(curr_pop);
		parent_2 = select_parent(curr_pop);
		if (randomDouble() < CROSSOVER_PROB) //if crossover prob. allows us to do the crossover
		{
			//selecting the crossover point within min op_count of two parents
			crossover = randMax( findmin(solutions[curr_pop][parent_1].op_count,solutions[curr_pop][parent_2].op_count) );
		} 
		else 
		{
			crossover = NUM_OPERATIONS; //otherwise set crossover point after the last plan index. So no crossover
		}
		for (j = 0 ; j < NUM_OPERATIONS ; j++) 
		{
			if (j < crossover) 
			{
				solutions[new_pop][i].plan[j] = mutate(solutions[curr_pop][parent_1].plan[j]);
				solutions[new_pop][i+1].plan[j] = mutate(solutions[curr_pop][parent_2].plan[j]);
			} 
			else 
			{
				solutions[new_pop][i].plan[j] = mutate(solutions[curr_pop][parent_2].plan[j]);
				solutions[new_pop][i+1].plan[j] = mutate(solutions[curr_pop][parent_1].plan[j]);
			}
		}
		solutions[new_pop][i].op_count = solutions[curr_pop][parent_1].op_count;
		solutions[new_pop][i+1].op_count = solutions[curr_pop][parent_2].op_count;
	}
	return new_pop;
}

void print_towers()
{
  	int peg, i;
	for (peg = 0 ; peg < 3 ; peg++) 
	{
		cout<<"Peg "<<peg<<" : ";
		for (i = 0 ; i < 3 ; i++) 
		{
      		cout<<pegs[peg].disc[i];
    	}
		cout<<endl;
	}

  	cout<<endl;
}

void trace_best( int curr )
{
  	int i, best;
  	double best_fitness = 0.0;
	for (i = 0 ; i < POPULATION_SIZE ; i++) 
	{

   		if (solutions[curr][i].fitness > best_fitness) 
   		{
      		best_fitness = solutions[curr][i].fitness;
      		best = i;
    	}

  	}

  	compute_fitness( curr, best, TRACE_ON );
}