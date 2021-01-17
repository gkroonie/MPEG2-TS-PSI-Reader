//MPEG2 Transport Stream Interrogator
//Written By George Kroon, 7-10/01/2018

#include <stdio.h>
#include <stdlib.h>
//The following line disables some deprecation warnings I was getting.
#pragma warning(disable : 4996)
#define PID_LENGTH 188

//Global Variables.
FILE *ts_file;
int bytes_read;
char byte_in; 
int pid_cnt = 0;
char pid_in[PID_LENGTH];
float pat_cnt = 0;
float pmt_cnt = 0;
int video_pid;
int audio_pid;
float video_cnt = 0;
float audio_cnt = 0;
float pat_perc;
float pmt_perc;
float video_perc;
float audio_perc;
int pid_number;
int ts_id;
int version_number;
int section_length;
int PCR_pid;
int es_number;
int pids[1000];
int unique = 0;
int unique_cnt = 0;
int pmt_pid;


/*Void function (not returning anything) to print out an example Programme Association Table.
Called in "int main", after working out the relevant figures. Removing the "if" statement
surrounding the printf functions will print all of the PATs. */
void PATsection(void) {
	if (pat_cnt == 1) {
		printf("\nAn example of the Program Association Table (PAT) has been found in packet number %d.\n", pid_cnt);
		printf("\nPAT No. %g:\n", pat_cnt);
		printf("\n-----------------------------------------------");
		printf("\nThe Section Length      | %d bytes", section_length);
		printf("\n-----------------------------------------------");
		printf("\nThe Transport Stream ID | 0x%04x", ts_id);
		printf("\n-----------------------------------------------");
		printf("\nThe Version Number      | 0x%02x", version_number);
		printf("\n-----------------------------------------------\n\n");
	}

	pmt_pid = ((pid_in[15] & 0x1f) << 8) + pid_in[16]; /*Working out the PID of the PMT from the "if-else" statement in the PAT.*/
}

/*Same as above, but to print out an example of the Programme Map Table. Also called in "int main". 
Removing the "if" statement surrounding the printf functions will print all of the PMTs. */
void PMTsection(void){
	PCR_pid = ((pid_in[13]&0x1f)<<8) + pid_in[14];
	es_number = (section_length - 13) / 5;
	
	if (pmt_cnt == 1) {
		printf("\n\nAn example of a Program Map Table (PMT) has been found in packet number %d.\n", pid_cnt);
		printf("\nPMT No. %g:\n", pmt_cnt);
		printf("\n-----------------------------------------------");
		printf("\nThe Version Number         | 0x%02x", version_number);
		printf("\n-----------------------------------------------");
		printf("\nThe PCR PID                | 0x%04x (%d)", PCR_pid, PCR_pid);
		printf("\n-----------------------------------------------");
		printf("\nNo. of Elementary Streams  | %d", es_number);
		printf("\n-----------------------------------------------\n\n\n");
	}

	/*The following is code to count through the number of elementary streams referenced within the pmt and 
	then find the pid for those elementary streams.*/
	int L = 0;

	for (int k = 0; k < es_number; k++) {
		L = k * 5;
		if (pid_in[17 + L] == 0x02) {
			video_pid = ((pid_in[18+ L] & 0x1f) << 8) + pid_in[19+ L];
		}

		if (pid_in[17 + L] == 0x03) {
			audio_pid = ((pid_in[18 + L] & 0x1f) << 8) + pid_in[19 + L];
		}
	}
}

/*Another void function to count the number of video and audio PIDs (given in brief's example) as well as 
counting and printing the unique PIDs in the transport stream. The is called and the end of "int main".*/
void PIDanalysis(void) {
	if (pid_number == video_pid) {
		video_cnt++;
	}

	if (pid_number == audio_pid) {
		audio_cnt++;
	}

	for (int i = 0; i < 1000; i++) {
		if (pid_number == pids[i]) {
			unique = 0;
		}
	}
	if (unique != 0) {
		pids[unique_cnt] = pid_number;
		unique_cnt++;
	}
	unique = 1;
}

/*This void function serves to print out all of the final figures, like the total number of PIDs, unique PIDs,
audio and video PIDs etc. and work out/print any relevant percentages.*/
void FINALstats(void) {
	pat_perc = 100 * pat_cnt / pid_cnt;
	pmt_perc = 100 * pmt_cnt / pid_cnt;
	video_perc = 100 * video_cnt / pid_cnt;
	audio_perc = 100 * audio_cnt / pid_cnt;

	printf("\n\n -------------\n");
	printf("| FINAL STATS |");
	printf("\n -------------");
	printf("\n\n\nTOTAL NUMBER OF PIDS: %d\n\n", pid_cnt);
	printf("\n\t~ TOTAL NUMBER OF PATS: %g\n", pat_cnt);
	printf("\t\tPAT to Total PID percentage: %.3f%% \n", pat_perc);
	printf("\n\t~ TOTAL NUMBER OF PMTS: %g\n", pmt_cnt);
	printf("\t\tPMT to Total PID percentage: %.3f%% \n", pmt_perc);
	printf("\n\t~ TOTAL NUMBER OF VIDEO PIDS: %g\n", video_cnt);
	printf("\t\tVideo PID to Total PID percentage: %.3f%% \n", video_perc);
	printf("\n\t~ TOTAL NUMBER OF AUDIO PIDS: %g\n", audio_cnt);
	printf("\t\tAudio PID to total PID percentage: %.3f%% \n", audio_perc);
	printf("\n\t~ The number of unique PIDs is: %d\n", (unique_cnt+1));
	//The following for loop counts through the "pids" array to print any values in there.
	for (int j = 0; j <= unique_cnt; j++) {
		printf("\t\t\tUnique PID %d:0x%04x (%d) \n", (j+1), pids[j], pids[j]);
	}
	printf("\n\t~ PID Number 0x%04x (%d) is the PCR Reference.\n\n", PCR_pid, PCR_pid);
}

/*This is the main function, controlling the order in which everything is called and runs, and returning 0 at the end,
to finish the program. It contains the title and intro, lots of working out different PIDs etc. using bitwise operators,
 and calls the different funtions (seen above) under the appropriate conditions.*/
int main() {

	printf("\n GEORGE KROON'S MPEG-2 TRANSPORT STREAM INTERROGATOR\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("\n\nThe purpose of this program is to be able to analyse an MPEG-2 transport stream, \nand identify its various contents, such as the Program Association Table, or the \nProgram Map Tables, and give meaning to the data contained within the packets.\n\n");

	//Opening the transport stream file.
	ts_file = fopen("D:\\Final Piece coding\\TransportStream-20171204.ts", "rb");
	if (ts_file != NULL)
	{ 
		//Parse through the file.
		do {
			bytes_read = fread(pid_in, sizeof(char), PID_LENGTH, ts_file);
			pid_cnt++;

			//Bitwise operations to find specific bit-fields, which correlate to various sections of the tables in the transport stream.
			section_length = (pid_in[6] & 0x0f) + pid_in[7];
			
			ts_id = pid_in[8] + pid_in[9];

			version_number = pid_in[10] & 0x3E;

			//Bitwise for the actual PIDs of the packets.
			char upper = pid_in[1];
			char lower = pid_in[2];
			pid_number = upper & 0x1f;
			pid_number = (pid_number<<8) + lower;


			//"If" statements, counting the PATs or PMTs when they are found, and calling the relevant functions if their PID comes up.
			if (pid_number == 0x0000) {
				pat_cnt++;
				PATsection();
			} 

			if (pid_number == pmt_pid) {
				pmt_cnt++;
				PMTsection();
			}

			//Calling the "PIDanalysis" function to run, counting audio, video, and unique PIDs.
			PIDanalysis();

		} while (bytes_read == PID_LENGTH);

		//Calling the "FINALstats" function to print a culmination of the analysis as a final list of figures.
		FINALstats();

		fclose(ts_file); /*Closing the transport stream file, so we are no longer reading data from it.*/
	}
	else { printf("\nFile not opened correctly.\n"); } /*The negative result of my "if" statement error check for opening the file.*/

	getchar();/*This function allows the results to print to the screen. */

	return 0; /*The main function returns 0 to end the program.*/
}
 
