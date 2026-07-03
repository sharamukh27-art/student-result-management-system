#ifndef STUDENT_H
#define STUDENT_H

#include <stdio.h>

// ─── Version ──────────────────────────────────────────────────────────────────
#define VERSION       "2.0"
#define FILE_VERSION  20          // bump this whenever struct layout changes

// ─── Constants ────────────────────────────────────────────────────────────────
#define MAX_STUDENTS  100
#define NUM_SUBJECTS  5
#define DATA_FILE     "students.dat"
#define REPORT_FILE   "report.txt"

// ─── ANSI Color macros ────────────────────────────────────────────────────────
#define COL_RESET   "\033[0m"
#define COL_BOLD    "\033[1m"
#define COL_RED     "\033[1;31m"
#define COL_GREEN   "\033[1;32m"
#define COL_YELLOW  "\033[1;33m"
#define COL_BLUE    "\033[1;34m"
#define COL_CYAN    "\033[1;36m"
#define COL_WHITE   "\033[1;37m"

// ─── Subject names (defined in student.c) ────────────────────────────────────
extern const char *SUBJECT_NAMES[NUM_SUBJECTS];

// ─── Struct ───────────────────────────────────────────────────────────────────
typedef struct {
    int   roll;
    char  name[50];
    int   marks[NUM_SUBJECTS];
    int   total;
    float percentage;
    char  grade;
    char  division[20];     // "Outstanding" / "First Division" / etc.
    int   rank;             // assigned at display time
    char  addedOn[30];      // timestamp: "03 Jul 2026  08:42 AM"
} Student;

// ─── Global storage (defined in student.c) ───────────────────────────────────
extern Student students[MAX_STUDENTS];
extern int     count;

// ─── Function Declarations ────────────────────────────────────────────────────

// Core CRUD
void addStudent();
void displayStudents();
void updateStudent();
void deleteStudent();

// Search
void searchMenu();
void searchByRoll();
void searchByName();

// Sort
void sortMenu();
void sortByName();
void sortByRoll();
void sortByPercentage();

// Analytics
void classStatistics();
void subjectToppers();
void gradeDistribution();
void displayMeritList();

// Export / Backup
void exportReport();
void backupData();

// Persistence
void saveToFile();
void loadFromFile();

// Info screens
void showHelp();
void showAbout();

// Utility
void calculateResult(Student *s);
void printReportCard(const Student *s);
void printReportCardToFile(FILE *fp, const Student *s);
void printBanner();
void assignRanks();
int  readValidInt(const char *prompt, int min, int max);
void readValidName(const char *prompt, char *out, int maxLen);
void getCurrentTimestamp(char *buf, int bufSize);

#endif /* STUDENT_H */
