#pragma once

class Sim
{
public:
    Sim();
    ~Sim();

    bool connect();
    void disconnect();
    [[nodiscard]] bool connected() const { return connected_; }

    // update data
    void process(); 
    
private:
    bool connected_;
};