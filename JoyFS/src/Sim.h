#pragma once

class Sim
{
public:
    Sim();
    ~Sim();

    bool connect();
    void disconnect();
    [[nodiscard]] bool connected() const { return connected_; }

private:
    bool connected_;
};