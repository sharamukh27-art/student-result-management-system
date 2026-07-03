#include <stdio.h>
#include "student.h"

int main()
{
    loadFromFile();
    printBanner();

    if (count > 0)
        printf(COL_GREEN "  %d student record(s) loaded.\n" COL_RESET, count);

    while (1)
    {
        printf(COL_BLUE "\n+=====================================================+\n" COL_RESET);
        printf(COL_BLUE "|       STUDENT RESULT MANAGEMENT SYSTEM             |\n" COL_RESET);
        printf(COL_BLUE "+=====================================================+\n" COL_RESET);
        printf("|  1.  Add Student                                    |\n");
        printf("|  2.  Display All Students                           |\n");
        printf("|  3.  Search Student (by Roll / Name)                |\n");
        printf("|  4.  Update Student Record                          |\n");
        printf("|  5.  Delete Student Record                          |\n");
        printf("|  6.  Subject Toppers & Averages                     |\n");
        printf("|  7.  Grade Distribution                             |\n");
        printf("|  8.  Class Statistics                               |\n");
        printf("|  9.  Merit List                                     |\n");
        printf("| 10.  Sort Students                                  |\n");
        printf("| 11.  Export Report to File                          |\n");
        printf("| 12.  Backup Data                                    |\n");
        printf("| 13.  Help                                           |\n");
        printf("| 14.  About                                          |\n");
        printf("| 15.  Exit                                           |\n");
        printf(COL_BLUE "+-----------------------------------------------------+\n" COL_RESET);

        int choice = readValidInt("  Enter choice (1-15): ", 1, 15);

        switch (choice)
        {
            case  1: addStudent();        break;
            case  2: displayStudents();   break;
            case  3: searchMenu();        break;
            case  4: updateStudent();     break;
            case  5: deleteStudent();     break;
            case  6: subjectToppers();    break;
            case  7: gradeDistribution(); break;
            case  8: classStatistics();   break;
            case  9: displayMeritList();  break;
            case 10: sortMenu();          break;
            case 11: exportReport();      break;
            case 12: backupData();        break;
            case 13: showHelp();          break;
            case 14: showAbout();         break;
            case 15:
                printf(COL_GREEN "\nData saved. Goodbye!\n\n" COL_RESET);
                return 0;
        }
    }
}
