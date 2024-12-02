#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <string>
#include <vector>
#include <fstream>
#include <ros/ros.h>
#include <boost/filesystem.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

class CSVLogger
{
public:
    CSVLogger(const std::string& workspace, const std::string& package, const std::string& filename, const std::vector<std::string>& header)
    {
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm* now_tm = std::localtime(&now);
        std::ostringstream timestamp;
        timestamp << std::put_time(now_tm, "%Y%m%d_%H%M");

        // Construct the absolute path for the CSV file with timestamp
        const char* home_dir = getenv("HOME");
        if (home_dir != nullptr)
        {
            std::string base_path = std::string(home_dir) + "/" + workspace + "/csvLogs/" + package + "/" + timestamp.str();
            csv_path_ = base_path + "/" + filename + ".csv";
            ROS_INFO("CSV file will be saved in %s", csv_path_.c_str());

            // Create directories if they do not exist
            boost::filesystem::create_directories(base_path);

            // Open CSV file
            csv_file_.open(csv_path_);
            if (!csv_file_.is_open())
            {
                ROS_ERROR("Failed to open CSV file at %s", csv_path_.c_str());
            }
            else
            {
                // Write the header to the CSV file
                csv_file_ << "timestamp,topic";
                for (const auto& col : header)
                {
                    csv_file_ << "," << col;
                }
                csv_file_ << "\n";
            }
        }
        else
        {
            ROS_ERROR("Failed to get HOME environment variable");
        }
    }

    ~CSVLogger()
    {
        if (csv_file_.is_open())
        {
            csv_file_.close();
        }
    }

    void writeCSV(const std::string& topic, const std::vector<double>& data)
    {
        if (csv_file_.is_open())
        {
            for (const auto& value : data)
            {
                csv_file_ << "," << value;
            }
            csv_file_ << "\n";
        }
    }

private:
    std::string csv_path_;
    std::ofstream csv_file_;
};

#endif // CSV_LOGGER_H