#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <cmath>
#include <fstream>
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include <limits>
#include "../dep/jsoncpp/jsoncpp.cpp" // relies on jsoncpp at https://github.com/open-source-parsers/jsoncpp

typedef std::map<std::string, std::tuple<std::string, int>> gpaHashMapStruc;

const std::string version = "0.1.0";
Json::Reader reader;
Json::Value root;
const std::string jsonFile = "gpa.json";
// based on https://www.nyp.edu.sg/current-students/academic-matters/nyp-assessment-regulations.html
const std::map<std::string, float> gpaRef = {
    {"DIST", 4.0}, // awarded by the Assessment Board
    {"A", 4.0},
    {"B+", 3.5},
    {"B", 3.0},
    {"C+", 2.5},
    {"C", 2.0},
    {"D+", 1.5},
    {"D", 1.0},
    {"F", 0.0},
    {"P", 0.0} // p for pass (GSM)
};

std::string uppercaseInput(std::string s)
{
    int i = 0;
    for (auto &c : s) {
        s[i] = toupper(c);
        i++;
    }
    return s;
}

std::string titleInput(std::string s)
{
    int toUpper = 1; int i = 0;
    for (auto &c : s) {
        if (toUpper) {
            s[i] = toupper(c);
            toUpper = 0;
        } else if(isspace(s[i])) {
            s[i] = tolower(c);
            toUpper = 1;
        }
        i++;
    }
    return s;
}

bool checkIfInputIsInt(std::string s)
{
    for (auto &c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool checkIfInputIsAlphabets(std::string s)
{
    for (auto &c : s) {
        if (!isalpha(c)) return false;
    }
    return true;
}

void pEnd(int numOfTimes = 1) 
{ 
    for (int i = 0; i < numOfTimes; i++) { 
        std::cout << "\n"; 
    }
}

void shutdown() 
{ 
    pEnd();
    std::cout << "Thank you for using GPA Calculator v" << version;
    pEnd();
    std::cout << "Please press ENTER to continue..."; 
    std::cin.get();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 3; i >= 0; i--) { 
        std::cout << "Automatically shutting down in " << i << " seconds...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (i != 0) std::cout << "\r";
    }
}

bool checkIfFileExist (const std::string& fileName) {
    struct stat buffer;   
    return (stat (fileName.c_str(), &buffer) == 0); 
}

void saveToPC(gpaHashMapStruc &oldGPAMap)
{   
    Json::Value newGPAMap;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "    ";

    for (gpaHashMapStruc::const_iterator it = oldGPAMap.begin(); it != oldGPAMap.end(); ++it) {
        newGPAMap["gpa"][it->first]["grade"] = std::get<0>(it->second);
        newGPAMap["gpa"][it->first]["credit"] = std::get<1>(it->second);
    }

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream out(jsonFile);
    writer->write(newGPAMap, &out);
}

float gradeToFloat(std::string grade) 
{
    grade = uppercaseInput(grade);
    auto it = gpaRef.find(grade);
    if ( it != gpaRef.end()) return it->second;
    else return -1.0;
}

bool checkIfInputIsValidGrade(std::string grade)
{
    grade = uppercaseInput(grade);
    int gradeLen = grade.length();
    if (gradeLen == 4) {
        if (grade != "DIST") return false;
    } else if (gradeLen == 2) {
        std::string charString = "";
        charString += grade[0];
        if (!checkIfInputIsAlphabets(charString)) return false;
        if (gradeLen == 2) {
            if (grade[1] != '+') return false;
        }
    } else if (gradeLen != 1) return false;

    auto it = gpaRef.find(grade);
    if ( it != gpaRef.end()) return true;
    else return false;
}

void printMsgWithNthPrec(const std::vector<std::string>& msgArr, int prec)
{
    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(prec);
    for (int i = 0; i < msgArr.size(); i++) {
        try {
            float num = std::stof(msgArr[i]);
            std::cout << num;
        } catch (std::invalid_argument& e) {
            std::cout << msgArr[i];
        }
    }
    pEnd();
}

void printMenu(float gpa, bool validJsonFile)
{
    std::cout << "\n\n------------ Menu ------------\n\n";
    std::string gpaString;
    if (gpa < 0) gpaString = "N/A";
    else gpaString = std::to_string(gpa);

    std::vector<std::string> msgArr = { "> Current GPA: ", gpaString };
    printMsgWithNthPrec(msgArr, 2);
    pEnd();

    int i = 1;
    std::cout << i << ". Add new module results";
    i++;
    if (validJsonFile) {
        pEnd();
        std::cout << i << ". Edit module results";
        i++;
        pEnd();

        std::cout << i << ". Remove module results";
        i++;
        pEnd();

        std::cout << i << ". View all module results";
        i++;
    }
    pEnd();

    std::cout << "F. Shutdown";
    pEnd();
    std::cout << "\n-------------------------------";
}

void readJsonGPAData(gpaHashMapStruc gpaMap)
{
    pEnd();
    std::cout << "----------------------------------------------\n\n";
    std::cout << "Reading GPA data in the format,\n\n";
    for(gpaHashMapStruc::const_iterator it = gpaMap.begin(); it != gpaMap.end(); it++) {
        std::string moduleName = it->first;
        std::tuple<std::string, int> value = it->second;
        std::cout << moduleName << " (" << std::get<1>(value) << "): " << std::get<0>(value) << "\n";
    }
    pEnd();
    std::cout << "Format:\nModule Name (Max Credits): Your Grade\n";
    std::cout << "\n----------------------------------------------\n";
}

float calculateGPA(gpaHashMapStruc gpaMap)
{
    int totalCredits = 0;
    int totalGrade = 0;
    for(gpaHashMapStruc::const_iterator it = gpaMap.begin(); it != gpaMap.end(); it++) {
        std::string grade = std::get<0>(it->second);
        if (grade != "P") {
            int credits = std::get<1>(it->second);
            totalCredits += credits;
            totalGrade += gradeToFloat(grade) * credits;
        }
    }
    return totalGrade / (float)totalCredits;
}

void mainProcess()
{
    bool jsonFileExist = checkIfFileExist(jsonFile);
    bool jsonValid = false;
    if (jsonFileExist) {
        std::ifstream file(jsonFile);
        bool parsed = reader.parse(file, root);
        if (!parsed){
            std::cout << "\nError: Cannot parse json content...\n";
        } else {
            if (root["gpa"].isNull()) {
                jsonValid = false;
                std::cout << "\nError: gpa.json does not have the necessary information...\n";
            } else jsonValid = true;
        }
    }
    gpaHashMapStruc gpaMap;
    float totalGPA = -1.0;
    if (jsonValid) {
        Json::Value values = root["gpa"];
        for(auto it = values.begin(); it != values.end(); it++) {
            std::string moduleName = it.key().asString();

            std::string grade = values[moduleName]["grade"].asString();
            int credit = values[moduleName]["credit"].asInt();

            gpaMap[moduleName] = std::make_tuple(grade, credit);
        }
        totalGPA = calculateGPA(gpaMap);
    } 

    std::string userInput = "";
    while (userInput != "F") {
        totalGPA = calculateGPA(gpaMap);
        printMenu(totalGPA, jsonValid);
        std::cout << "\nPlease enter your desired command: "; std::getline(std::cin, userInput);
        userInput = uppercaseInput(userInput);
        
        if (userInput == "1") {
            // add new module results
            bool continueAdding = true;
            while (continueAdding) {
                std::string finalModuleName;
                int finalCredit = 0;
                std::string finalGrade;

                // adding the name of the module
                while (continueAdding) {
                    std::string moduleName;
                    bool addedModuleName = false;
                    std::cout << "\nPlease enter the module name (x to cancel): "; std::getline(std::cin, moduleName);
                    moduleName = titleInput(moduleName);
                    if (moduleName == "X") {
                        continueAdding = false;
                        break;
                    }
                    if (gpaMap.find(moduleName) != gpaMap.end()) {
                        std::cout << "Error: Module name already exists...\n";
                    } else if (!moduleName.empty()) {
                        while (1) {
                            std::cout << "Are you sure that you would to add the module with the name, " << moduleName << "? (y/n): ";
                            std::string confirmInput;
                            std::getline(std::cin, confirmInput);
                            confirmInput = uppercaseInput(confirmInput);
                            if (confirmInput == "Y") {
                                finalModuleName = moduleName;
                                addedModuleName = true;
                                break;
                            } else if (confirmInput == "N") {
                                break;
                            } else {
                                std::cout << "Invalid input...\n";
                            }
                        }
                    } else {
                        std::cout << "Error: Input cannot be empty...\n";
                    }
                    
                    if (addedModuleName) break;
                }

                // adding the grade obtained by the user for that module
                while (continueAdding) {
                    std::string moduleGrade;
                    bool addedGrade = false;
                    std::cout << "\nPlease enter your grade for \"" << finalModuleName << "\" (x to cancel): "; std::getline(std::cin, moduleGrade);
                    moduleGrade = uppercaseInput(moduleGrade);
                    if (moduleGrade == "X") {
                        continueAdding = false;
                        break;
                    }
                    if (checkIfInputIsValidGrade(moduleGrade)) {
                        while (1) {
                            std::cout << "Are you sure that you would to add your grade, \"" << moduleGrade << "\", to the module, " << finalModuleName << "? (y/n): ";
                            std::string confirmInput;
                            std::getline(std::cin, confirmInput);
                            confirmInput = uppercaseInput(confirmInput);
                            if (confirmInput == "Y") {
                                finalGrade = moduleGrade;
                                addedGrade = true;
                                break;
                            } else if (confirmInput == "N") {
                                break;
                            } else {
                                std::cout << "Invalid input...\n";
                            }
                        }
                    } else {
                        std::cout << "Invalid grade input...\n";
                    }
                    
                    if (addedGrade) break;
                }

                // adding the module credits
                while (continueAdding) {
                    std::string moduleCredit;
                    bool addedCredit = false;
                    std::cout << "\nPlease enter the number of credits for \"" << finalModuleName << "\" (x to cancel): "; 
                    std::getline(std::cin, moduleCredit);
                    if (uppercaseInput(moduleCredit) == "X") {
                        continueAdding = false;
                        break;
                    }
                    if (checkIfInputIsInt(moduleCredit)) {
                        int moduleCreditNum = std::stoi(moduleCredit);
                        if (moduleCreditNum >= 0 || moduleCreditNum <= 99) {
                            while (1) {
                                std::string confirmInput;
                                std::cout << "Are you sure that you would to add the module credits of, " << moduleCredit << ", for the module, " << finalModuleName << "? (y/n): "; 
                                std::getline(std::cin, confirmInput);
                                confirmInput = uppercaseInput(confirmInput);
                                if (confirmInput == "Y") {
                                    finalCredit = moduleCreditNum;
                                    addedCredit = true;
                                    break;
                                } else if (confirmInput == "N") {
                                    break;
                                } else {
                                    std::cout << "Invalid input...\n";
                                }
                            }
                        } else {
                            std::cout << "Credit input is either too high or is in the negative range...\n";
                        }
                    } else {
                        std::cout << "Invalid input...\n";
                    }

                    if (addedCredit) break;
                }

                if (continueAdding) {
                    std::cout << "\nAdded module, " << finalModuleName << ", with grade, " << finalGrade << ", and credits, " << finalCredit << ", to gpa.json...\n";
                    gpaMap[finalModuleName] = std::make_tuple(finalGrade, finalCredit);
                    saveToPC(gpaMap);
                    std::cout << "------------------------------------------------------------------------------------\n";
                    jsonValid = true;
                }
            }

        } else if (userInput == "2" && jsonValid) {
            // edit existing module result
            bool readGPA = true;
            while (1) {
                if (readGPA) readJsonGPAData(gpaMap);
                std::string moduleToEdit;
                std::cout << "\nPlease enter the module name that you would to edit (X to cancel): "; std::getline(std::cin, moduleToEdit);
                moduleToEdit = titleInput(moduleToEdit);
                if (moduleToEdit == "X") break;

                if (gpaMap.find(moduleToEdit) != gpaMap.end()) {
                    bool editedInfo = false;
                    std::string initialModName = moduleToEdit;
                    std::string initialGrade = std::get<0>(gpaMap[moduleToEdit]);
                    int initialCredit = std::get<1>(gpaMap[moduleToEdit]);

                    while (1) {
                        std::cout << "----------------------------------------------\n\n";
                        std::cout << "Currently editing " << moduleToEdit << "...\n";
                        std::cout << "Current grade: " << std::get<0>(gpaMap[moduleToEdit]) << "\n";
                        std::cout << "Current credit: " << std::get<1>(gpaMap[moduleToEdit]) << "\n\n";
                        std::cout << "Commands:\n";
                        std::cout << "\"n\" to edit the module name\n";
                        std::cout << "\"g\" to edit the grade\n";
                        std::cout << "\"c\" to edit the credit\n";
                        if (editedInfo) {
                            std::cout << "\"s\" to save changes\n";
                            std::cout << "\"b\" to revert changes\n";
                        }
                        std::cout << "\"x\" to cancel editing and return to menu\n";
                        std::cout << "\n----------------------------------------------\n";
                        
                        std::string editCommand;
                        std::cout << "Enter command: "; std::getline(std::cin, editCommand);
                        editCommand = uppercaseInput(editCommand);
                        if (editCommand == "X") break;

                        else if (editCommand == "N") {
                            while (1) {
                                std::string newModuleName;
                                std::cout << "Enter new module name (x to cancel): "; std::getline(std::cin, newModuleName);
                                newModuleName = titleInput(newModuleName);
                                if (gpaMap.find(newModuleName) != gpaMap.end()) {
                                    std::cout << "Error: Module name already exists...\n";
                                } else if (newModuleName == "X") {
                                    break;
                                } else if (!newModuleName.empty()) {
                                    gpaMap[newModuleName] = gpaMap[moduleToEdit];
                                    gpaMap.erase(moduleToEdit);
                                    moduleToEdit = newModuleName;
                                    editedInfo = true;
                                    break;
                                } else {
                                    std::cout << "Error: Input cannot be empty...\n";
                                }
                            }
                        } else if (editCommand == "G") {
                            while (1) {
                                std::string newGrade;
                                std::cout << "Enter new grade (x to cancel): "; std::getline(std::cin, newGrade);
                                newGrade = uppercaseInput(newGrade);
                                if (newGrade == "X") {
                                    break;
                                } else if (!checkIfInputIsValidGrade(newGrade)) {
                                    std::cout << "Error: Invalid grade...\n";
                                } else {
                                    std::get<0>(gpaMap[moduleToEdit]) = newGrade;
                                    editedInfo = true;
                                    break;
                                }
                            }

                        } else if (editCommand == "C") {
                            while (1) {
                                std::string newCredit;
                                std::cout << "Enter new credit (x to cancel): "; std::getline(std::cin, newCredit);
                                if (checkIfInputIsInt(newCredit)) {
                                    int newCreditVal = std::stoi(newCredit);
                                    if (newCreditVal >= 0 && newCreditVal <= 99) {
                                        std::get<1>(gpaMap[moduleToEdit]) = newCreditVal;
                                        editedInfo = true;
                                        break;
                                    } else {
                                        std::cout << "Error: Credit input is either too high or is in the negative range...\n";
                                    }
                                } else if (uppercaseInput(newCredit) == "X") {
                                    break;
                                } else {
                                    std::cout << "Error: Invalid credit input or credit input was larger than 99...\n";
                                }
                            }
                            

                        } else if (editCommand == "S" && editedInfo) {
                            std::cout << "Are you sure that you would to save the changes? (y/n): ";
                            std::string confirmSave;
                            std::getline(std::cin, confirmSave);
                            confirmSave = uppercaseInput(confirmSave);
                            if (confirmSave == "Y") {
                                saveToPC(gpaMap);
                                editedInfo = false;
                            } else if (confirmSave == "N") {
                                std::cout << "Saving of changes aborted...\n";
                            } else {
                                std::cout << "Error: Invalid input...\n";
                            }

                        } else if (editCommand == "B" && editedInfo) {
                            std::cout << "Are you sure that you would to revert the changes? (y/n): ";
                            std::string confirmSave;
                            std::getline(std::cin, confirmSave);
                            confirmSave = uppercaseInput(confirmSave);
                            if (confirmSave == "Y") {
                                gpaMap.erase(moduleToEdit);
                                gpaMap[initialModName] = std::make_tuple(initialGrade, initialCredit);
                                moduleToEdit = initialModName;
                                editedInfo = false;
                            } else if (confirmSave == "N") {
                                std::cout << "Reverting of changes aborted...\n";
                            } else {
                                std::cout << "Error: Invalid input...\n";
                            }

                        } else {
                            std::cout << "Invalid command. Please enter a valid command from the menu above.\n\n";
                        }
                    }
                    break;
                } else {
                    std::cout << "Module not found!\n";
                    readGPA = false;
                }
            }

        } else if (userInput == "3" && jsonValid) {
            // remove module result
            bool readGPA = true;
            while (1) {
                if (readGPA) readJsonGPAData(gpaMap);
                std::string moduleToRemove;
                std::cout << "\nPlease enter the module name that you would like to remove (X to cancel): "; std::getline(std::cin, moduleToRemove);
                moduleToRemove = titleInput(moduleToRemove);
                if (moduleToRemove == "X") break;
                if (gpaMap.find(moduleToRemove) != gpaMap.end()) {
                    std::string confirmErase;
                    while (1) {
                        std::cout << "Are you sure you want to remove the module, " << moduleToRemove << "? (Y/N): "; 
                        std::getline(std::cin, confirmErase);
                        confirmErase = uppercaseInput(confirmErase);
                        if (confirmErase == "Y" || confirmErase == "N") break;
                        else std::cout << "Please enter \"Y\" or \"N\"\n";
                    }
                    
                    if (confirmErase == "Y") {
                        gpaMap.erase(moduleToRemove);
                        std::cout << "Module " << moduleToRemove << " has been removed.\n";
                        saveToPC(gpaMap);
                    } else std::cout << "Module " << moduleToRemove << " will NOT be removed.\n";
                    readGPA = true;
                } else {
                    std::cout << "Module not found!\n";
                    readGPA = false;
                }
            }

        } else if (userInput == "4" && jsonValid) {
            // read all module results
            readJsonGPAData(gpaMap);

        } else if (userInput != "F") { 
            // shutdown
            std::cout << "Invalid command input, please enter a valid command from the menu above.\n";
        } 
    }
}

int main() 
{   
    std::cout << "========================== GPA Calculator v" << version << " ==========================\n";
    std::cout << "================ https://github.com/KJHJason/GPACalculator ================\n";
    std::cout << "============================ Author: KJHJason =============================\n";
    std::cout << "============================== License: MIT ===============================\n";
    try {
        mainProcess();
    } catch(const std::runtime_error& re) {
        std::cout << "\nRuntime error encountered: " << re.what();
        pEnd();
        shutdown();
        return 1;
    } catch (const std::exception& e) {
        std::cout << "\nError encountered: " << e.what();
        pEnd();
        shutdown();
        return 1;
    } catch(...) {
        std::cout << "\nUnknown error encountered.";
        pEnd();
        shutdown();
        return 1;
    }
    shutdown();
    return 0;
}