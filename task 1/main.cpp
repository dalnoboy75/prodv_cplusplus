#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>

class Group;

class User : public std::enable_shared_from_this<User> {
public:
    User(int id, std::string name) : id_(id), name_(std::move(name)) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    std::shared_ptr<Group> group() const { return group_.lock(); }

    void set_group(const std::shared_ptr<Group>& group);
    void remove_group();

private:
    int id_;
    std::string name_;
    std::weak_ptr<Group> group_;
};

class Group {
public:
    explicit Group(int id) : id_(id) {}

    int id() const { return id_; }
    std::vector<std::shared_ptr<User>> users() const;

    void add_user(const std::shared_ptr<User>& user);
    void remove_user(const std::shared_ptr<User>& user);

private:
    int id_;
    std::vector<std::weak_ptr<User>> users_;
};

class UserManager {
public:
    void create_user(int user_id, const std::string& name, int group_id = -1);
    void delete_user(int user_id);
    void create_group(int group_id);
    void delete_group(int group_id);
    void print_all_users() const;
    void print_user(int user_id) const;
    void print_all_groups() const;
    void print_group(int group_id) const;

private:
    std::map<int, std::shared_ptr<User>> users_;
    std::map<int, std::shared_ptr<Group>> groups_;

    std::shared_ptr<Group> find_group(int group_id) const;
    std::shared_ptr<User> find_user(int user_id) const;
};


void User::set_group(const std::shared_ptr<Group>& group) {
    if (auto current_group = group_.lock()) {
        current_group->remove_user(shared_from_this());
    }

    group_ = group;
    if (group) {
        group->add_user(shared_from_this());
    }
}

void User::remove_group() {
    if (auto g = group_.lock()) {
        g->remove_user(shared_from_this());
    }
    group_.reset();
}


std::vector<std::shared_ptr<User>> Group::users() const {
    std::vector<std::shared_ptr<User>> valid_users;
    for (const auto& user : users_) {
        if (auto u = user.lock()) {
            valid_users.push_back(u);
        }
    }
    return valid_users;
}

void Group::add_user(const std::shared_ptr<User>& user) {
    users_.emplace_back(user);
}

void Group::remove_user(const std::shared_ptr<User>& user) {
    users_.erase(
            std::remove_if(users_.begin(), users_.end(),
                           [&](const auto& weak_user) {
                               auto u = weak_user.lock();
                               return u->id() == user->id();
                           }),
            users_.end());
}

void UserManager::create_user(int user_id, const std::string& name, int group_id) {
    if (users_.count(user_id)) {
        std::cout << "Error: User " << user_id << " already exists\n";
        return;
    }
    if (group_id != -1 && find_group(group_id) == nullptr) {
        std::cout << "Error: Group " << group_id << " not found\n";
        return;
    }

    auto user = std::make_shared<User>(user_id, name);
    users_[user_id] = user;

    if (group_id != -1) {
        if (auto group = find_group(group_id)) {
            user->set_group(group);
        }
    }
}

void UserManager::delete_user(int user_id) {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
        std::cout << "Error: User " << user_id << " not found\n";
        return;
    }

    it->second->remove_group();
    users_.erase(it);
    std::cout << "User " << user_id << " deleted\n";
}

void UserManager::create_group(int group_id) {
    if (groups_.count(group_id)) {
        std::cout << "Error: Group " << group_id << " already exists\n";
        return;
    }

    groups_[group_id] = std::make_shared<Group>(group_id);
    std::cout << "Group " << group_id << " created\n";
}

void UserManager::delete_group(int group_id) {
    auto it = groups_.find(group_id);
    if (it == groups_.end()) {
        std::cout << "Error: Group " << group_id << " not found\n";
        return;
    }

    auto group = it->second;
    for (auto& user : group->users()) {
        user->remove_group();
    }

    groups_.erase(it);
    std::cout << "Group " << group_id << " deleted\n";
}

void UserManager::print_all_users() const {
    if (users_.empty()) {
        std::cout << "No users found\n";
        return;
    }

    for (const auto& [id, user] : users_) {
        print_user(id);
    }
}

void UserManager::print_user(int user_id) const {
    if (auto user = find_user(user_id)) {
        std::cout << "User ID: " << user_id
                  << "\nName: " << user->name()
                  << "\nGroup: ";
        if (auto g = user->group()) {
            std::cout << g->id();
        } else {
            std::cout << "None";
        }
        std::cout << "\n-------------\n";
    } else {
        std::cout << "Error: User " << user_id << " not found\n";
    }
}

void UserManager::print_all_groups() const {
    if (groups_.empty()) {
        std::cout << "No groups found\n";
        return;
    }

    for (const auto& [id, group] : groups_) {
        print_group(id);
    }
}

void UserManager::print_group(int group_id) const {
    if (auto group = find_group(group_id)) {
        std::cout << "Group ID: " << group_id << "\nMembers:";
        auto members = group->users();
        if (members.empty()) {
            std::cout << " None";
        } else {
            for (const auto& user : members) {
                std::cout << "\n- " << user->name() << " (ID: " << user->id() << ")";
            }
        }
        std::cout << "\n-------------\n";
    } else {
        std::cout << "Error: Group " << group_id << " not found\n";
    }
}

std::shared_ptr<Group> UserManager::find_group(int group_id) const {
    auto it = groups_.find(group_id);
    return it != groups_.end() ? it->second : nullptr;
}

std::shared_ptr<User> UserManager::find_user(int user_id) const {
    auto it = users_.find(user_id);
    return it != users_.end() ? it->second : nullptr;
}

int main() {
    UserManager manager;
    std::string line;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit") break;

        try {
            if (cmd == "createUser") {
                int id, group = -1;
                std::string name;
                iss >> id >> name;
                if (iss >> group) {}
                manager.create_user(id, name, group);
            }
            else if (cmd == "deleteUser") {
                int id;
                iss >> id;
                manager.delete_user(id);
            }
            else if (cmd == "allUsers") {
                manager.print_all_users();
            }
            else if (cmd == "getUser") {
                int id;
                iss >> id;
                manager.print_user(id);
            }
            else if (cmd == "createGroup") {
                int id;
                iss >> id;
                manager.create_group(id);
            }
            else if (cmd == "deleteGroup") {
                int id;
                iss >> id;
                manager.delete_group(id);
            }
            else if (cmd == "allGroups") {
                manager.print_all_groups();
            }
            else if (cmd == "getGroup") {
                int id;
                iss >> id;
                manager.print_group(id);
            }
            else {
                std::cout << "Unknown command\n";
            }
        } catch (...) {
            std::cout << "Invalid command format\n";
        }
    }
}
