#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <cmath>
#include <fstream>
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include "../dep/json/jsoncpp.cpp"

Json::Reader reader;
Json::Value root;
const std::string jsonFile = "gpa.json";
// based on https://www.nyp.edu.sg/current-students/academic-matters/nyp-assessment-regulations.html
const std::map<std::string, float> nypGpaRef = {
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

bool checkIfFileExist (const std::string& fileName) {
    struct stat buffer;   
    return (stat (fileName.c_str(), &buffer) == 0); 
}

void saveToPC(std::map<std::string, int> &inMap)
{
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "    ";
    std::map<std::string, int> arr[1];
    arr[0] = inMap;

    std::cout << "Saving GPA Info...\n";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream(jsonFile);
    writer -> write(arr, &outputFileStream);
}

int gradeToFloat(std::string grade) 
{
    auto it = nypGpaRef.find(grade);
    if ( it != nypGpaRef.end()) return it->second;
    else {
        std::cout << "Error: " << grade << " is not a valid grade.\n";
        std::cout << "Suggested solution: Please manually edit gpa.json and change the grade to a valid grade\n";
        return -1;
    }
}

float round2dp(float value)
{
    return round(value * 100.0) / 100.0;
}

int main() 
{   
    
    bool jsonFileExist = checkIfFileExist(jsonFile);
    bool jsonValid = false;
    if (jsonFileExist) {
        std::ifstream file(jsonFile);
        bool parsed = reader.parse(file, root);
        if (!parsed){
            std::cout << "Error: Cannot parse json content...";
        } else {
            if (root["gpa"].isNull()) {
                jsonValid = false;
                std::cout << "Error: gpa.json does not have the necessary information...";
            } else jsonValid = true;
        }
    }
    std::map<std::string, std::tuple<std::string, int>>gpaMap;
    if (jsonValid) {
        double totalGPA = 0.0;
        float modPtProductCreditVal = 0.0;
        int totalModCredVal = 0;

        Json::Value values = root["gpa"];
        for(int j = 0; j < values[0].getMemberNames().size(); j++) {
            std::string key = values[0].getMemberNames()[j];
            Json::Value hmAccessor = values[0][values[0].getMemberNames()[j]];

            std::string grade = hmAccessor["grade"].asString();
            int credits = hmAccessor["credit"].asInt();

            gpaMap[key] = std::make_tuple(grade, credits);
            modPtProductCreditVal += gradeToFloat(grade) * credits;
            totalModCredVal += credits;
        }
        totalGPA = modPtProductCreditVal / totalModCredVal;

        std::cout << "Welcome back! Currently, your GPA is " << round2dp(totalGPA) << "\n";
    } else {
        std::cout << "Welcome! Please enter your NYP GPA: ";
        std::string nypGpa;
        std::cin >> nypGpa;
        std::cout << "Please enter your NYP course: ";
        std::string nypCourse;
        std::cin >> nypCourse;
        
    }
    
    return 0;
}