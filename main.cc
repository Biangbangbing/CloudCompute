#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <pthread.h>
#include <string>
#include <cstring>
#include <thread>
#include <sys/time.h>
#include <iostream>

#include "ThreadPool.h"
#include "ThreadPool1.h"
#include "sudoku.h"

using namespace std;

const int puzzle_max = 1800; //每次可以处理的最大数独行

int** ques; //存储数独题目的数组
bool* is_solved; //记录是否解决当前行题目的数组
char puzzle[128];
int situation;
int line_size;
int line_no = 0;
bool file_end = false;

int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

void one_round_init() {
	for (int i = 0; i < puzzle_max; i++) {
		is_solved[i] = false;
	}
}

void transform(const char in[N], int* out) {
	for (int i = 0; i < N; i++) {
		out[i] = in[i] - '0';
	}
}

void Worker() {
	for (int i = 0; i < line_size; i++) {
		is_solved[i] = solve_sudoku_dancing_links(ques[situation + i]);
	}
}

void Worker2(ifstream & input_file) {
    for (line_no; line_no < puzzle_max; line_no++) {
	if (input_file.eof()) {
	        //cout<<"结束了"<<endl;
		file_end = true;
		line_no-=1;
		//cout<<"inside:only"<<file_end<<"   "<<line_no<<endl;
		return;
		//cout<<"inside:never"<<file_end<<"   "<<line_no<<endl;
	}
	//cout<<"inside:"<<file_end<<"   "<<line_no<<endl;
	input_file.getline(puzzle, N + 1);
	if (strlen(puzzle) >= N)
	{
		transform(puzzle, ques[line_no]);
	}
   }
}


int main() {
	ios::sync_with_stdio(false);
	ques = new int* [puzzle_max];
	is_solved = new bool[puzzle_max];
	for (int i = 0; i < puzzle_max; i++) {
		ques[i] = new int[N];
	}
	string file_path;
	while (getline(cin, file_path)) {
		//FILE *fout;
		//fout = fopen("byAnswer1000.txt","wt");
		int64_t start = now();   //开始计时
		ifstream input_file(file_path, ios::in);
		if (!input_file.is_open())
		{
			printf("Filed to open file: %s\n", file_path.c_str());
			return 0;
		}
		one_round_init();
		file_end = false;
		while (!file_end) {
			line_no = 0;
			//TaskEntry* task = (TaskEntry*)malloc(sizeof(TaskEntry))(input_file);
			//cout<<"1:"<<(line_no)<<endl;
			TaskEntry* task1 = new TaskEntry(input_file);
			//task->args = input_file;
			task1->func = Worker2;
			ThreadPool_i poolin(4);
			poolin.start();
			poolin.run(task1);
			poolin.stop();
			//cout<<"2:"<<(line_no)<<endl;
			//if (file_end)
			//    break;
			//int64_t end = now();
			/*int line_no = 0;
			while (line_no < puzzle_max) {
				if (input_file.eof()) {
					file_end = true;
					break;
				}
				input_file.getline(puzzle, N + 1);
				if (strlen(puzzle) >= N)
				{
					transform(puzzle, ques[line_no]);
					line_no++;
				}
			}*/
			situation = 0;
			line_size = line_no;
			ThreadPool pool(4);
			pool.start();
			pool.run(Worker);
			pool.stop();
			//cout<<"3:"<<(line_no)<<endl;
			for (int i = 0; i < line_no; i++) {
				if (is_solved[i]) {
					for (int j = 0; j < N; j++) {
						putchar('0' + ques[i][j]);
						//fprintf(fout,"%c",'0' +ques[i][j]);
					}
					putchar('\n');
					//fprintf(fout,"%c",'\n');
				}
				else {
					puts("No results.");
				}
			}
			
			//fclose(fout);
		}
		input_file.close();
		//fclose(fout);
		int64_t end = now();   //结束计时
		double sec = (end-start)/1000000.0;
		printf("%f sec %f ms each %d\n", sec, 1000*sec/10000, 10000);
	}
	for (int i = 0; i < puzzle_max; i++) {
		delete[] ques[i];
	}
	delete[]ques;
	return 0;
}
