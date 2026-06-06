#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <string>
#include <vector>
#include <variant>
#include <fstream>
#include <boost/filesystem.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <mutex>

class CSVLogger
{
public:
    CSVLogger(const std::string& workspace, const std::string& package, const std::string& filename, const std::vector<std::string>& header)
    {
        static_cast<void>(package);
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm* now_tm = std::localtime(&now);
        std::ostringstream timestamp;
        timestamp << std::put_time(now_tm, "%Y%m%d_%H%M");

        // Construct the absolute path for the CSV file with timestamp.
        // All packages now write directly into the timestamp directory so each
        // experiment keeps a flat CSV set.
        const char* home_dir = getenv("HOME");
        if (home_dir != nullptr)
        {
            std::string base_path = std::string(home_dir) + "/" + workspace + "/csvLogs/" + timestamp.str();
            csv_path_ = base_path + "/" + filename + ".csv";
            std::cout << "[CSVLogger] CSV file will be saved in " << csv_path_ << std::endl;

            // Create directories if they do not exist
            boost::filesystem::create_directories(base_path);

            // Open CSV file
            csv_file_.open(csv_path_);
            if (!csv_file_.is_open())
            {
                std::cerr << "[CSVLogger] Failed to open CSV file at " << csv_path_ << std::endl;
            }
            else
            {
                // Write header as a full line, then flush.
                std::ostringstream row;
                for (const auto& col : header)
                {
                    row << "," << col;
                }
                write_line_locked_(row.str());
            }
        }
        else
        {
            std::cerr << "[CSVLogger] Failed to get HOME environment variable" << std::endl;
        }
    }

    ~CSVLogger()
    {
        std::lock_guard<std::mutex> lock(csv_mutex_);
        if (csv_file_.is_open())
        {
            csv_file_.flush();
            csv_file_.close();
        }
    }

    void writeCSV(const std::vector<std::variant<std::string, double>>& data)
    {
        std::ostringstream row;
        for (const auto& value : data)
        {
            std::visit([&](auto&& arg) { row << "," << arg; }, value);
        }

        std::lock_guard<std::mutex> lock(csv_mutex_);
        write_line_locked_(row.str());
    }

private:
    void write_line_locked_(const std::string& row)
    {
        if (!csv_file_.is_open())
        {
            return;
        }

        csv_file_ << row << "\n";
        // Ensures last complete line reaches the file even on normal Ctrl+C shutdown.
        csv_file_.flush();

        if (!csv_file_.good())
        {
            std::cerr << "[CSVLogger] Write failed for " << csv_path_ << std::endl;
        }
    }

    std::string csv_path_;
    std::ofstream csv_file_;
    std::mutex csv_mutex_;
};

#endif // CSV_LOGGER_H
