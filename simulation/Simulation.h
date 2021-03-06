//
// Created by benji on 08/11/16.
//

#ifndef LANGTON3D_SIMULATION_H
#define LANGTON3D_SIMULATION_H

#include <unistd.h>
#include <vector>

#include "Elements/Ant.h"
#include "Grid3D.h"
#include "Message_Colors.h"
#include "Rendering/Context.h"
#include "Rendering/Scene.h"
#include "Rendering/Window.h"

#define DEFAULT_UPDATE_FREQUENCY 100 // Hz
#define LIMIT_SIMULATION 1500 // divergence of the ant after

class EventListener {
public:
    void init(Window * mainWindow);

    virtual void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) = 0;
    virtual void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) = 0;

    void setEventListener(EventListener* eventListener);

    static EventListener *listener;
protected:
    void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};


class Simulation : public EventListener {

public:
    /**
     * @param ruleID from 1 to PRE_CONFIGURED_RULES_NUMBER
     */
    Simulation(int ruleID = 1);
    Simulation(RuleDefinition rules);
    ~Simulation();

    void addAnt(int x, int y, int z);
    void addAnt(Vector3 position);

    void start();

    void initialize();

    void createRules();
    void setRules(int ruleID);
    void setRules(RuleDefinition rules);

    void printHelp();

protected:
    void mainLoop();

    void input();

    void createWindow();
    void createControlKeys();

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

private:
    std::unique_ptr<Scene> _scene = nullptr;

    std::unique_ptr<Window> _window = nullptr;
    std::shared_ptr<Grid3D> _grid = nullptr;
    std::shared_ptr<Rules> _rules = nullptr;

    std::vector<Vector3> _antsPosition;
    std::vector<std::unique_ptr<Ant>> _listAnts;

    int _count = 0;
    int _currentPreConfiguredRules = 0;

    double _beginSimulation;
    double _beginSimulationPaused = 0;
    void pauseSimulation(bool desactivate);
    std::vector<int> _checkpointsList;
    void addCheckpoints();

    bool _pauseSimulation = false;
    bool _pauseDisplaying = false;

    // Controls
    int _rightKey;
    int _leftKey;
    int _upKey;
    int _downKey;

    int _keyA; // +Y
    int _keyZ; // +Z
    int _keyE; // -Y
    int _keyQ; // -X
    int _keyS; // -Z
    int _keyD; // +X

    void centerCamera();

    void emptyBuffer() {
        cin.clear();
        cin.seekg(0, ios::end);

        if (cin.eof())
            cin.ignore(numeric_limits<streamsize>::max());
        else
            cin.clear();
    }
};


#endif //LANGTON3D_SIMULATION_H
