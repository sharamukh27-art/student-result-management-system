#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "student.h"

const char *SUBJECT_NAMES[NUM_SUBJECTS] = {
    "Mathematics", "Physics", "Chemistry", "English", "Computer Science"
};

Student students[MAX_STUDENTS];
int     count = 0;

/* ─── Utility ─────────────────────────────────────────────────────────── */

void getCurrentTimestamp(char *buf, int bufSize) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) { buf[0] = '\0'; return; }
    strftime(buf, bufSize, "%d %b %Y  %I:%M %p", t);
}

void printBanner() {
    printf("\n");
    printf(COL_CYAN "**********************************************\n" COL_RESET);
    printf(COL_CYAN "*                                            *\n" COL_RESET);
    printf(COL_BOLD COL_WHITE "     STUDENT RESULT MANAGEMENT SYSTEM\n" COL_RESET);
    printf(COL_YELLOW "              Version %s\n" COL_RESET, VERSION);
    printf(COL_CYAN "*                                            *\n" COL_RESET);
    printf(COL_CYAN "**********************************************\n" COL_RESET);
    printf("\n");
}

int readValidInt(const char *prompt, int min, int max) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) != 1) {
            while (getchar() != '\n');
            printf(COL_RED "  Invalid! Enter a number between %d and %d.\n" COL_RESET, min, max);
            continue;
        }
        /* FIX: flush the rest of the line so leftover chars (e.g. "5x")
           can't leak into the next scanf, especially %[^\n] name reads */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }

        if (value < min || value > max) {
            printf(COL_RED "  Invalid! Enter a value between %d and %d.\n" COL_RESET, min, max);
            continue;
        }
        return value;
    }
}

void readValidName(const char *prompt, char *out, int maxLen) {
    /* FIX: cap scanf's read width to maxLen-1 so a long name can never
       overflow `out`. Building the format string with the width baked in. */
    char fmt[24];
    snprintf(fmt, sizeof(fmt), " %%%d[^\n]", maxLen - 1);

    while (1) {
        printf("%s", prompt);
        if (scanf(fmt, out) != 1) {
            out[0] = '\0';
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { /* discard overflow/leftover */ }

        if (strlen(out) == 0) {
            printf(COL_RED "  Name cannot be empty.\n" COL_RESET); continue;
        }
        if ((int)strlen(out) >= maxLen) {
            printf(COL_RED "  Name too long (max %d chars).\n" COL_RESET, maxLen - 1); continue;
        }
        int hasLetter = 0;
        for (int i = 0; out[i]; i++)
            if ((out[i] >= 'A' && out[i] <= 'Z') || (out[i] >= 'a' && out[i] <= 'z')) { hasLetter = 1; break; }
        if (!hasLetter) {
            printf(COL_RED "  Name must contain letters.\n" COL_RESET); continue;
        }
        return;
    }
}

/* ─── Result Calculation ──────────────────────────────────────────────── */

void calculateResult(Student *s) {
    s->total = 0;
    for (int i = 0; i < NUM_SUBJECTS; i++) s->total += s->marks[i];
    s->percentage = s->total / (float)NUM_SUBJECTS;

    if      (s->percentage >= 90) s->grade = 'A';
    else if (s->percentage >= 80) s->grade = 'B';
    else if (s->percentage >= 70) s->grade = 'C';
    else if (s->percentage >= 60) s->grade = 'D';
    else                          s->grade = 'F';

    /* FIX: strncpy + explicit NUL termination instead of strcpy, so a
       future change to these labels can never silently overflow division[20] */
    const char *div;
    if      (s->percentage >= 90) div = "Outstanding";
    else if (s->percentage >= 75) div = "First Division";
    else if (s->percentage >= 60) div = "Second Division";
    else if (s->percentage >= 50) div = "Third Division";
    else                          div = "Fail";
    strncpy(s->division, div, sizeof(s->division) - 1);
    s->division[sizeof(s->division) - 1] = '\0';
}

/* FIX: assignRanks() used to physically re-sort the master `students[]`
   array by percentage every time it ran — and it ran inside addStudent,
   updateStudent, deleteStudent, loadFromFile, AND displayStudents(). That
   meant sortByName()/sortByRoll() got silently undone the moment anything
   called displayStudents() right after, because it re-sorted by percentage
   before printing. Now assignRanks() only computes each student's rank
   field; it never reorders the array. Physical reordering is left to the
   explicit sort functions, so the array stays in whatever order the user
   last asked for. */
void assignRanks() {
    if (count == 0) return;

    int *order = malloc(count * sizeof(int));
    if (!order) return;
    for (int i = 0; i < count; i++) order[i] = i;

    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - 1 - i; j++)
            if (students[order[j]].percentage < students[order[j + 1]].percentage) {
                int tmp = order[j]; order[j] = order[j + 1]; order[j + 1] = tmp;
            }

    int rank = 1;
    for (int i = 0; i < count; i++) {
        if (i > 0 && students[order[i]].percentage == students[order[i - 1]].percentage)
            students[order[i]].rank = students[order[i - 1]].rank;
        else
            students[order[i]].rank = rank;
        rank++;
    }
    free(order);
}

/* ─── Report Cards ────────────────────────────────────────────────────── */

void printReportCardToFile(FILE *fp, const Student *s) {
    fprintf(fp, "+--------------------------------------------------+\n");
    fprintf(fp, "|             STUDENT REPORT CARD                  |\n");
    fprintf(fp, "+--------------------------------------------------+\n");
    fprintf(fp, "| Roll Number : %-34d |\n", s->roll);
    fprintf(fp, "| Name        : %-34s |\n", s->name);
    if (s->rank > 0)   fprintf(fp, "| Rank        : %-34d |\n", s->rank);
    if (s->addedOn[0]) fprintf(fp, "| Added On    : %-34s |\n", s->addedOn);
    fprintf(fp, "+--------------------------------------------------+\n");
    for (int j = 0; j < NUM_SUBJECTS; j++)
        fprintf(fp, "| %-18s : %-29d |\n", SUBJECT_NAMES[j], s->marks[j]);
    fprintf(fp, "+--------------------------------------------------+\n");
    fprintf(fp, "| Total       : %-34d |\n", s->total);
    fprintf(fp, "| Percentage  : %-33.2f%% |\n", s->percentage);
    fprintf(fp, "| Grade       : %-34c |\n", s->grade);
    fprintf(fp, "| Division    : %-34s |\n", s->division);
    fprintf(fp, "+--------------------------------------------------+\n");
}

void printReportCard(const Student *s) {
    const char *rc = (s->grade == 'A' || s->grade == 'B') ? COL_GREEN
                    : (s->grade == 'F')                    ? COL_RED : COL_YELLOW;
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    printf(COL_BLUE "|" COL_RESET COL_BOLD "             STUDENT REPORT CARD                  " COL_RESET COL_BLUE "|\n" COL_RESET);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    printf(COL_BLUE "| " COL_RESET "Roll Number : %-34d" COL_BLUE " |\n" COL_RESET, s->roll);
    printf(COL_BLUE "| " COL_RESET "Name        : " COL_BOLD "%-34s" COL_RESET COL_BLUE " |\n" COL_RESET, s->name);
    if (s->rank > 0)
        printf(COL_BLUE "| " COL_RESET "Rank        : " COL_YELLOW "%-34d" COL_RESET COL_BLUE " |\n" COL_RESET, s->rank);
    if (s->addedOn[0])
        printf(COL_BLUE "| " COL_RESET "Added On    : %-34s" COL_BLUE " |\n" COL_RESET, s->addedOn);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    for (int j = 0; j < NUM_SUBJECTS; j++)
        printf(COL_BLUE "| " COL_RESET "%-18s : %-29d" COL_BLUE " |\n" COL_RESET, SUBJECT_NAMES[j], s->marks[j]);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    printf(COL_BLUE "| " COL_RESET "Total       : %-34d"        COL_BLUE " |\n" COL_RESET, s->total);
    printf(COL_BLUE "| " COL_RESET "Percentage  : %s%-33.2f%%" COL_RESET COL_BLUE " |\n" COL_RESET, rc, s->percentage);
    printf(COL_BLUE "| " COL_RESET "Grade       : %s%-34c"     COL_RESET COL_BLUE " |\n" COL_RESET, rc, s->grade);
    printf(COL_BLUE "| " COL_RESET "Division    : %s%-34s"     COL_RESET COL_BLUE " |\n" COL_RESET, rc, s->division);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
}

/* ─── Core CRUD ───────────────────────────────────────────────────────── */

void addStudent() {
    if (count >= MAX_STUDENTS) { printf(COL_RED "\nStorage full!\n" COL_RESET); return; }
    Student s; memset(&s, 0, sizeof(Student));
    while (1) {
        s.roll = readValidInt("\nEnter Roll Number (1-9999): ", 1, 9999);
        int dup = 0;
        for (int i = 0; i < count; i++)
            if (students[i].roll == s.roll) { dup = 1; break; }
        if (dup) { printf(COL_RED "  Roll %d already exists!\n" COL_RESET, s.roll); continue; }
        break;
    }
    readValidName("Enter Name        : ", s.name, sizeof(s.name));
    printf("\nEnter marks for %d subjects (0-100):\n", NUM_SUBJECTS);
    for (int i = 0; i < NUM_SUBJECTS; i++) {
        char p[60]; snprintf(p, sizeof(p), "  %-18s : ", SUBJECT_NAMES[i]);
        s.marks[i] = readValidInt(p, 0, 100);
    }
    calculateResult(&s);
    getCurrentTimestamp(s.addedOn, sizeof(s.addedOn));
    students[count++] = s;
    assignRanks(); saveToFile();
    printf(COL_GREEN "\nStudent added successfully!\n" COL_RESET);
    printf(COL_YELLOW "  Record Created: %s\n" COL_RESET, s.addedOn);
}

void displayStudents() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    assignRanks(); printf("\n");
    for (int i = 0; i < count; i++) printReportCard(&students[i]);
}

void updateStudent() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    int roll = readValidInt("\nEnter Roll Number to update (1-9999): ", 1, 9999);
    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            Student *s = &students[i];
            printf("\nCurrent record:\n"); printReportCard(s);
            printf("\n  1. Name  2. Marks  3. Both\n");
            int ch = readValidInt("Enter choice (1-3): ", 1, 3);
            if (ch == 1 || ch == 3) readValidName("Enter new name: ", s->name, sizeof(s->name));
            if (ch == 2 || ch == 3) {
                printf("\nEnter new marks (0-100):\n");
                for (int j = 0; j < NUM_SUBJECTS; j++) {
                    char p[60]; snprintf(p, sizeof(p), "  %-18s : ", SUBJECT_NAMES[j]);
                    s->marks[j] = readValidInt(p, 0, 100);
                }
            }
            calculateResult(s); assignRanks(); saveToFile();
            printf(COL_GREEN "\nRecord updated!\n" COL_RESET); printReportCard(s);
            return;
        }
    }
    printf(COL_RED "\nNo student found with Roll Number %d.\n" COL_RESET, roll);
}

void deleteStudent() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    int roll = readValidInt("\nEnter Roll Number to delete (1-9999): ", 1, 9999);
    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            printf("\nRecord to delete:\n"); printReportCard(&students[i]);
            printf("Are you sure? (y/n): ");
            char c; scanf(" %c", &c);
            if (c != 'y' && c != 'Y') { printf(COL_YELLOW "Deletion cancelled.\n" COL_RESET); return; }
            for (int j = i; j < count - 1; j++) students[j] = students[j + 1];
            count--; assignRanks(); saveToFile();
            printf(COL_GREEN "\nStudent deleted!\n" COL_RESET); return;
        }
    }
    printf(COL_RED "\nNo student found with Roll Number %d.\n" COL_RESET, roll);
}

/* ─── Search ──────────────────────────────────────────────────────────── */

void searchByRoll() {
    int roll = readValidInt("\nEnter Roll Number to search (1-9999): ", 1, 9999);
    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            printf(COL_GREEN "\nStudent Found!\n" COL_RESET);
            printReportCard(&students[i]); return;
        }
    }
    printf(COL_RED "\nNo student found with Roll Number %d.\n" COL_RESET, roll);
}

void searchByName() {
    char name[50]; readValidName("\nEnter Name to search: ", name, sizeof(name));
    int found = 0;
    for (int i = 0; i < count; i++) {
        char hay[50], ndl[50]; int k;
        for (k = 0; students[i].name[k] && k < (int)sizeof(hay) - 1; k++)
            hay[k] = (students[i].name[k] >= 'A' && students[i].name[k] <= 'Z')
                     ? students[i].name[k] + 32 : students[i].name[k];
        hay[k] = '\0';
        for (k = 0; name[k] && k < (int)sizeof(ndl) - 1; k++)
            ndl[k] = (name[k] >= 'A' && name[k] <= 'Z') ? name[k] + 32 : name[k];
        ndl[k] = '\0';
        if (strstr(hay, ndl)) {
            if (!found) printf(COL_GREEN "\nSearch Results:\n" COL_RESET);
            printReportCard(&students[i]); found++;
        }
    }
    if (!found) printf(COL_RED "\nNo student found with name containing \"%s\".\n" COL_RESET, name);
}

void searchMenu() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    printf("\nSearch Student\n  1. By Roll Number\n  2. By Name\n");
    int ch = readValidInt("Enter choice (1-2): ", 1, 2);
    if (ch == 1) searchByRoll(); else searchByName();
}

/* ─── Sort ────────────────────────────────────────────────────────────── */

void sortByName() {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(students[j].name, students[j + 1].name) > 0) {
                Student t = students[j]; students[j] = students[j + 1]; students[j + 1] = t;
            }
    saveToFile(); printf(COL_GREEN "\nSorted by Name.\n" COL_RESET);
    for (int i = 0; i < count; i++) printReportCard(&students[i]);
}

void sortByRoll() {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (students[j].roll > students[j + 1].roll) {
                Student t = students[j]; students[j] = students[j + 1]; students[j + 1] = t;
            }
    saveToFile(); printf(COL_GREEN "\nSorted by Roll Number.\n" COL_RESET);
    for (int i = 0; i < count; i++) printReportCard(&students[i]);
}

void sortByPercentage() {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (students[j].percentage < students[j + 1].percentage) {
                Student t = students[j]; students[j] = students[j + 1]; students[j + 1] = t;
            }
    assignRanks(); saveToFile();
    printf(COL_GREEN "\nSorted by Percentage (highest first).\n" COL_RESET);
    for (int i = 0; i < count; i++) printReportCard(&students[i]);
}

void sortMenu() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    printf("\nSort Students\n  1. By Name\n  2. By Roll Number\n  3. By Percentage\n");
    int ch = readValidInt("Enter choice (1-3): ", 1, 3);
    if (ch == 1) sortByName(); else if (ch == 2) sortByRoll(); else sortByPercentage();
}

/* ─── Analytics ───────────────────────────────────────────────────────── */

void classStatistics() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    float hi = students[0].percentage, lo = students[0].percentage, sum = 0;
    int passed = 0;
    for (int i = 0; i < count; i++) {
        float p = students[i].percentage; sum += p;
        if (p > hi) hi = p;
        if (p < lo) lo = p;
        if (p >= 50) passed++;
    }
    float avg = sum / count; int failed = count - passed;
    printf(COL_BLUE "\n+--------------------------------------------------+\n" COL_RESET);
    printf(COL_BLUE "|               CLASS REPORT                       |\n" COL_RESET);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    printf("| Total Students     : %-27d |\n", count);
    printf("| Highest Percentage : " COL_GREEN  "%-27.2f" COL_RESET " |\n", hi);
    printf("| Lowest Percentage  : " COL_RED    "%-27.2f" COL_RESET " |\n", lo);
    printf("| Average Percentage : " COL_YELLOW "%-27.2f" COL_RESET " |\n", avg);
    printf("| Pass Count         : " COL_GREEN  "%-27d"   COL_RESET " |\n", passed);
    printf("| Fail Count         : " COL_RED    "%-27d"   COL_RESET " |\n", failed);
    printf("| Pass %%             : " COL_GREEN  "%-26.1f%%" COL_RESET " |\n", (passed / (float)count) * 100.0f);
    printf("| Fail %%             : " COL_RED    "%-26.1f%%" COL_RESET " |\n", (failed / (float)count) * 100.0f);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
}

void subjectToppers() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    printf(COL_BLUE "\n+--------------------------------------------------+\n" COL_RESET);
    printf(COL_BLUE "|              SUBJECT TOPPERS                      |\n" COL_RESET);
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        int ti = 0, hi = students[0].marks[j], sum = 0;
        for (int i = 0; i < count; i++) {
            sum += students[i].marks[j];
            if (students[i].marks[j] > hi) { hi = students[i].marks[j]; ti = i; }
        }
        printf("| %-18s | Topper: " COL_GREEN "%-12s" COL_RESET " | Score: " COL_YELLOW "%3d" COL_RESET " |\n",
               SUBJECT_NAMES[j], students[ti].name, hi);
        printf("|                   | Class Avg : " COL_CYAN "%-23.2f" COL_RESET " |\n", sum / (float)count);
        printf("|                   +----------------------------------|\n");
    }
    printf(COL_BLUE "+--------------------------------------------------+\n" COL_RESET);
}

void gradeDistribution() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    int a = 0, b = 0, c = 0, d = 0, f = 0;
    for (int i = 0; i < count; i++) {
        switch (students[i].grade) {
            case 'A': a++; break; case 'B': b++; break;
            case 'C': c++; break; case 'D': d++; break; default: f++; break;
        }
    }
    printf(COL_BLUE "\n+---------------------------+\n" COL_RESET);
    printf(COL_BLUE "|    GRADE DISTRIBUTION     |\n" COL_RESET);
    printf(COL_BLUE "+---------------------------+\n" COL_RESET);
    printf("| A Grade (90-100)      : " COL_GREEN  "%-2d" COL_RESET " |\n", a);
    printf("| B Grade (80-89)       : " COL_GREEN  "%-2d" COL_RESET " |\n", b);
    printf("| C Grade (70-79)       : " COL_YELLOW "%-2d" COL_RESET " |\n", c);
    printf("| D Grade (60-69)       : " COL_YELLOW "%-2d" COL_RESET " |\n", d);
    printf("| F Grade (Below 60)    : " COL_RED    "%-2d" COL_RESET " |\n", f);
    printf(COL_BLUE "+---------------------------+\n" COL_RESET);
}

void displayMeritList() {
    if (count == 0) { printf(COL_YELLOW "\nNo student records found.\n" COL_RESET); return; }
    Student sorted[MAX_STUDENTS]; memcpy(sorted, students, count * sizeof(Student));
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (sorted[j].percentage < sorted[j + 1].percentage) {
                Student t = sorted[j]; sorted[j] = sorted[j + 1]; sorted[j + 1] = t;
            }
    printf(COL_BLUE "\n+------+--------------------------------+------------+-------+\n" COL_RESET);
    printf(COL_BLUE "| Rank | Name                           | Percentage | Grade |\n" COL_RESET);
    printf(COL_BLUE "+------+--------------------------------+------------+-------+\n" COL_RESET);
    for (int i = 0; i < count; i++) {
        const char *cl = (sorted[i].grade == 'A' || sorted[i].grade == 'B') ? COL_GREEN
                        : (sorted[i].grade == 'F') ? COL_RED : COL_YELLOW;
        printf("| %-4d | %-30s | %s%9.2f%%" COL_RESET " | %s%-5c" COL_RESET " |\n",
               i + 1, sorted[i].name, cl, sorted[i].percentage, cl, sorted[i].grade);
    }
    printf(COL_BLUE "+------+--------------------------------+------------+-------+\n" COL_RESET);
}

/* ─── Export / Backup ─────────────────────────────────────────────────── */

void exportReport() {
    if (count == 0) { printf(COL_YELLOW "\nNo records to export.\n" COL_RESET); return; }
    FILE *fp = fopen(REPORT_FILE, "w");
    if (!fp) { printf(COL_RED "\nCould not create %s.\n" COL_RESET, REPORT_FILE); return; }
    Student sorted[MAX_STUDENTS]; memcpy(sorted, students, count * sizeof(Student));
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (sorted[j].percentage < sorted[j + 1].percentage) {
                Student t = sorted[j]; sorted[j] = sorted[j + 1]; sorted[j + 1] = t;
            }
    for (int i = 0; i < count; i++) sorted[i].rank = i + 1;
    char ts[30]; getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "STUDENT RESULT MANAGEMENT SYSTEM v%s -- FULL REPORT\n", VERSION);
    fprintf(fp, "Generated : %s\nTotal Students : %d\n\n", ts, count);
    for (int i = 0; i < count; i++) printReportCardToFile(fp, &sorted[i]);
    fprintf(fp, "\n========== MERIT LIST ==========\n");
    fprintf(fp, "%-4s  %-30s  %-10s  %-5s\n", "Rank", "Name", "Percentage", "Grade");
    fprintf(fp, "%-4s  %-30s  %-10s  %-5s\n", "----", "----", "----------", "-----");
    for (int i = 0; i < count; i++)
        fprintf(fp, "%-4d  %-30s  %9.2f%%  %-5c\n",
                sorted[i].rank, sorted[i].name, sorted[i].percentage, sorted[i].grade);
    fclose(fp);
    printf(COL_GREEN "\nReport exported to '%s'.\n" COL_RESET, REPORT_FILE);
}

void backupData() {
    if (count == 0) { printf(COL_YELLOW "\nNo records to back up.\n" COL_RESET); return; }
    time_t now = time(NULL); struct tm *t = localtime(&now);
    char fn[48];
    if (t) {
        /* FIX: added H-M-S so multiple backups on the same day don't
           silently overwrite each other */
        snprintf(fn, sizeof(fn), "backup_%02d_%02d_%04d_%02d%02d%02d.dat",
                 t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                 t->tm_hour, t->tm_min, t->tm_sec);
    } else {
        snprintf(fn, sizeof(fn), "backup_unknown.dat");
    }
    FILE *fp = fopen(fn, "wb");
    if (!fp) { printf(COL_RED "\nCould not create '%s'.\n" COL_RESET, fn); return; }
    int ver = FILE_VERSION;
    fwrite(&ver, sizeof(int), 1, fp);
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(students, sizeof(Student), count, fp);
    fclose(fp);
    printf(COL_GREEN "\nBackup saved to '%s' (%d records).\n" COL_RESET, fn, count);
}

/* ─── Persistence ─────────────────────────────────────────────────────── */

void saveToFile() {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) { printf(COL_YELLOW "\nWarning: Could not save data.\n" COL_RESET); return; }
    int ver = FILE_VERSION;
    fwrite(&ver, sizeof(int), 1, fp);
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(students, sizeof(Student), count, fp);
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return;
    int ver = 0;
    if (fread(&ver, sizeof(int), 1, fp) != 1 || ver != FILE_VERSION) {
        fclose(fp); remove(DATA_FILE);
        printf(COL_YELLOW "\nNote: Old/incompatible data file detected and removed. Please re-enter records.\n" COL_RESET);
        return;
    }
    int loadedCount = 0;
    if (fread(&loadedCount, sizeof(int), 1, fp) != 1) { count = 0; fclose(fp); return; }
    if (loadedCount < 0 || loadedCount > MAX_STUDENTS) {
        printf(COL_RED "\nWarning: Corrupt data file. Starting fresh.\n" COL_RESET);
        count = 0; fclose(fp); return;
    }
    size_t loaded = fread(students, sizeof(Student), loadedCount, fp);
    count = (int)loaded;
    fclose(fp);
    if (count > 0) assignRanks();
}

/* ─── Info Screens ────────────────────────────────────────────────────── */

void showHelp() {
    printf(COL_CYAN "\n+=====================================================+\n" COL_RESET);
    printf(COL_CYAN "|                      HELP                          |\n" COL_RESET);
    printf(COL_CYAN "+=====================================================+\n" COL_RESET);
    printf("\n  This system allows you to:\n\n");
    printf(COL_GREEN "  + Add Students\n" COL_RESET);
    printf("      Roll number unique (1-9999). Marks 0-100.\n\n");
    printf(COL_GREEN "  + Search\n" COL_RESET);
    printf("      By roll number or partial name (case-insensitive).\n\n");
    printf(COL_GREEN "  + Update / Delete\n" COL_RESET);
    printf("      Update name, marks or both. Delete asks confirmation.\n\n");
    printf(COL_GREEN "  + Analytics\n" COL_RESET);
    printf("      Subject toppers, grade distribution, class stats, merit list.\n\n");
    printf(COL_GREEN "  + Export & Backup\n" COL_RESET);
    printf("      Export -> report.txt  |  Backup -> backup_DD_MM_YYYY_HHMMSS.dat\n\n");
    printf(COL_YELLOW "  Data is auto-saved to '%s' after every change.\n\n" COL_RESET, DATA_FILE);
    printf(COL_CYAN "+=====================================================+\n" COL_RESET);
}

void showAbout() {
    printf(COL_CYAN "\n+====================================+\n" COL_RESET);
    printf(COL_CYAN "|               ABOUT               |\n" COL_RESET);
    printf(COL_CYAN "+====================================+\n" COL_RESET);
    printf("|\n");
    printf("|  " COL_BOLD "Student Result Management System\n" COL_RESET);
    printf("|\n");
    printf("|  Version   : " COL_YELLOW "%s\n" COL_RESET, VERSION);
    printf("|  Language  : " COL_GREEN "C\n" COL_RESET);
    printf("|  Max Cap.  : " COL_GREEN "%d students / %d subjects\n" COL_RESET, MAX_STUDENTS, NUM_SUBJECTS);
    printf("|\n");
    printf(COL_CYAN "+====================================+\n" COL_RESET);
}