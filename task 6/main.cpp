#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>

class Checkpoint {
protected:
    std::string name;
    double latitude;
    double longitude;
    int sequence_number;

    Checkpoint(std::string name, double lat, double lon, int seq)
            : name(std::move(name)), latitude(validate_latitude(lat)),
              longitude(validate_longitude(lon)), sequence_number(validate_sequence(seq)) {}

public:
    virtual ~Checkpoint() = default;

    static double validate_latitude(double value) {
        if (value < -90.0 || value > 90.0) throw std::invalid_argument("Invalid latitude");
        return value;
    }

    static double validate_longitude(double value) {
        if (value < -180.0 || value > 180.0) throw std::invalid_argument("Invalid longitude");
        return value;
    }

    static int validate_sequence(int value) {
        if (value < 1) throw std::invalid_argument("Invalid sequence number");
        return value;
    }

    virtual double get_penalty() const = 0;
    virtual void print_info() const = 0;

    int sequence() const { return sequence_number; }
    const std::string& get_name() const { return name; }
};

class MandatoryCheckpoint : public Checkpoint {
public:
    MandatoryCheckpoint(std::string name, double lat, double lon, int seq)
            : Checkpoint(std::move(name), lat, lon, seq) {}

    double get_penalty() const override { return 0.0; }

    void print_info() const override {
        std::cout << sequence_number << ". " << name << "\n"
                  << "  Coordinates: " << latitude << " "
                  << longitude << "\n"
                  << "  Status: Mandatory\n\n";
    }
};


class OptionalCheckpoint : public Checkpoint {
    double penalty;

public:
    OptionalCheckpoint(std::string name, double lat, double lon, int seq, double penalty)
            : Checkpoint(std::move(name), lat, lon, seq), penalty(penalty) {
        if (penalty < 0) throw std::invalid_argument("Penalty cannot be negative");
    }

    double get_penalty() const override { return penalty; }

    void print_info() const override {
        std::cout << sequence_number << ". " << name << "\n"
                  << "  Coordinates: " << latitude << " "
                  << longitude << "\n"
                  << "  Penalty for skip: " << penalty << " hours\n\n";
    }
};

class CheckpointReportBuilder {
protected:
    std::vector<std::unique_ptr<Checkpoint>> checkpoints;

public:
    virtual ~CheckpointReportBuilder() = default;
    virtual void add_mandatory(std::string name, double lat, double lon, int seq) = 0;
    virtual void add_optional(std::string name, double lat, double lon, int seq, double penalty) = 0;
    virtual void generate() = 0;
};

class TextReportBuilder : public CheckpointReportBuilder {
public:
    void add_mandatory(std::string name, double lat, double lon, int seq) override {
        checkpoints.emplace_back(
                std::make_unique<MandatoryCheckpoint>(std::move(name), lat, lon, seq)
        );
    }

    void add_optional(std::string name, double lat, double lon, int seq, double penalty) override {
        checkpoints.emplace_back(
                std::make_unique<OptionalCheckpoint>(std::move(name), lat, lon, seq, penalty)
        );
    }

    void generate() override {
        std::sort(checkpoints.begin(), checkpoints.end(),
                  [](const auto& a, const auto& b) { return a->sequence() < b->sequence(); });

        std::cout << "Checkpoint list:\n";
        for (const auto& cp : checkpoints) {
            cp->print_info();
        }
    }
};

class PenaltyCalculator : public CheckpointReportBuilder {
    double total_penalty = 0.0;

public:
    void add_mandatory(std::string, double, double, int) override {}

    void add_optional(std::string, double, double, int, double penalty) override {
        total_penalty += penalty;
    }

    void generate() override {
        std::cout << "Total penalty for skipped optional checkpoints: "
                  << total_penalty << " hours\n";
    }
};

class RaceDirector {
public:
    void construct_race(CheckpointReportBuilder& builder) {
        builder.add_mandatory("Start", 55.7522, 37.6156, 1);
        builder.add_optional("Mountain Pass", 43.3560, 42.4395, 2, 2.5);
        builder.add_mandatory("River Crossing", 45.0355, 41.9697, 3);
        builder.add_optional("Desert Section", 44.8584, 34.1375, 4, 3.0);
        builder.add_mandatory("Finish", 55.7539, 37.6208, 5);
        builder.generate();
    }
};


int main() {
    RaceDirector director;

    TextReportBuilder text_builder;
    director.construct_race(text_builder);

    PenaltyCalculator penalty_calc;
    director.construct_race(penalty_calc);

    return 0;
}