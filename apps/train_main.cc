
/**
 * @file train_main.cc
 * @brief Runs tabular self-play training and saves learned Q-values.
 * @author Arian Mohammadian
 */

#include "poker/learning_agent.h"
#include "poker/q_table.h"
#include "poker/training_runner.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        if (argc > 4) {
            std::cerr
                << "Usage: " << argv[0]
                << " [episodes] [output_checkpoint] [input_checkpoint]\n";
            return 1;
        }

        poker::TrainingConfig config;

        // Use a small default for the first training experiment.
        config.episodes = 100;
        config.seed = 42;

        if (argc >= 2) {
            const std::string input = argv[1];

            if (input.empty() || input[0] == '-')
                throw std::invalid_argument("Invalid episode count");

            std::size_t parsed = 0;
            const auto episodes = std::stoull(input, &parsed);

            if (parsed != input.size() || episodes == 0)
                throw std::invalid_argument("Invalid episode count");

            config.episodes = episodes;
        }

        const std::string outputPath =
            argc >= 3 ? argv[2] : "checkpoints/tabular.qtable";

        if (outputPath.empty())
            throw std::invalid_argument("Empty output path");

        // Shared model.
        poker::QTable table;

        if (argc >= 4) {
            table.load(argv[3]);
            std::cout << "Loaded checkpoint: " << argv[3] << '\n';
        }

        // Independent random generators for both agents.
        std::mt19937 rng0{1234};
        std::mt19937 rng1{5678};

        poker::LearningConfig learningConfig;
        learningConfig.epsilon = 0.10;
        learningConfig.equitySamples = 50;
        learningConfig.bigBlind = config.bigBlind;

        poker::LearningAgent agent0(
            rng0, table, learningConfig
        );

        poker::LearningAgent agent1(
            rng1, table, learningConfig
        );

        std::cout
            << "\n====================================\n"
            << " POKER AI LAB - SELF-PLAY TRAINING\n"
            << "====================================\n"
            << "Algorithm: Tabular Monte Carlo Control\n"
            << "Episodes: " << config.episodes << '\n'
            << "Exploration: " << learningConfig.epsilon * 100.0 << "%\n"
            << "Postflop MC samples: " << learningConfig.equitySamples << '\n'
            << "Starting stack: " << config.startingStack << '\n'
            << "Big blind: " << config.bigBlind << '\n'
            << "Training seed: " << config.seed << '\n'
            << "\nTraining...\n";

        const poker::TrainingResult result =
            poker::TrainingRunner::run(
                agent0, agent1, config
            );

        if (result.episodesCompleted != config.episodes)
            throw std::runtime_error(
                "Training completed fewer episodes than requested"
            );

        // Create the destination directory when necessary.
        const std::filesystem::path path(outputPath);

        if (!path.parent_path().empty()) {
            std::filesystem::create_directories(
                path.parent_path()
            );
        }

        table.save(outputPath);

        std::cout
            << "\nTraining complete!\n"
            << "Episodes completed: "
            << result.episodesCompleted << '\n'
            << "Total decisions: "
            << result.totalDecisions << '\n'
            << "Checkpoint saved: "
            << outputPath << '\n';

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Training failed: " << e.what() << '\n';
        return 1;
    }
}
