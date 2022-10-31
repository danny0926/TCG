/**
 * Framework for Threes! and its variants (C++ 11)
 * agent.h: Define the behavior of variants of agents including players and environments
 *
 * Author: Theory of Computer Games
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <unistd.h>
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <fstream>
#include "board.h"
#include "action.h"
#include "weight.h"

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * base agent for agents with weight tables and a learning rate
 */
class weight_agent : public agent {
public:
	weight_agent(const std::string& args = "") : agent(args), alpha(0) {
		if (meta.find("init") != meta.end())
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end())
			load_weights(meta["load"]);
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~weight_agent() {
		if (meta.find("save") != meta.end())
			save_weights(meta["save"]);
	}

	int feature_pattern(board& after, std::vector<int> network) {
		/*int index = 0;
		double j = 0;
		double c = 15;
		std::vector<int>::iterator it;
		for(it = network.begin(); it != network.end(); ++it, ++j) {
			int board_number;
			if(after(*it) <= 3) board_number = after(*it);
			else {
				board_number = int(log2(after(*it) / 3)) + 3;
				//if (board_number > 10) board_number = 10;

			}

			index += board_number * int(std::pow(c, j));
		}


		return index;
		*/
		int c = 16;
		int a0 = network[0];
		int a1 = network[1];
		int a2 = network[2];
		int a3 = network[3];
		int a4 = network[4];
		int a5 = network[5];
		return after(a0)*c*c*c*c*c + after(a1)*c*c*c*c + after(a2)*c*c*c + after(a3)*c*c + after(a4)*c + after(a5);
	}
  	
	int feature_bignumber(board& after) {
		std::vector<int>network(6);
		board temp = after;
		for(int i = 0; i < 16; ++i) {
			switch(temp(i)) {
				case 192:
					++network[0];
					break;
				case 768:
					++network[1];
					break;
				case 1536:
					++network[2];
					break;
				case 3072:
					++network[3];
					break;
				case 6144:
					++network[4];
					break;
				case 12288:
					++network[5];
					break;
			}
		}
		
		int index = 0;
		double j = 0;
		double c = 16;
		std::vector<int>::iterator it;
		for(it = network.begin(); it != network.end(); ++it, ++j) {
			index += *it * int(std::pow(c, j));
		}

		return index;

	}
	

	float estimate_value(const board& after) {
		float value = 0;
		int rotate = 0;
		//int net_number = 0;
		board temp = after;
		std::vector<std::vector<int> >::iterator it;
	
		value += net[0][feature_bignumber(temp)];
		//++net_number;
		/*
		while(rotate < 4) {
			for(it = networks.begin(), net_number = 1; it != networks.end(); ++it, ++net_number) {
				value += net[net_number][feature_pattern(temp, *it)];
			}
			temp.reflect_horizontal();
			for(it = networks.begin(), net_number = 1; it != networks.end(); ++it, ++net_number) {
				value += net[net_number][feature_pattern(temp, *it)];
		       	}
			temp.reflect_horizontal();
			temp.rotate();
			rotate += 1;
		}*/

		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.reflect_horizontal();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		temp.rotate();
		value += net[1][feature_pattern(temp, net1)];
		value += net[2][feature_pattern(temp, net2)];
		value += net[3][feature_pattern(temp, net3)];
		value += net[4][feature_pattern(temp, net4)];
		
		return value;
	}
	void adjust_net_value(const board& after, float target) {
		float current = estimate_value(after);
		float error = target - current;
		float adjust = alpha * error;
		
		//int net_number = 0;
		int rotate = 0;
		board temp = after;
		std::vector<std::vector<int> >::iterator it;


		net[0][feature_bignumber(temp)] += adjust;
		//++net_number;
		/*
		while(rotate < 4) {
			for(it = networks.begin(), net_number = 1; it != networks.end(); ++it, ++net_number) {
				net[net_number][feature_pattern(temp, *it)] += adjust;
			}
			temp.reflect_horizontal();
			for(it = networks.begin(), net_number = 1; it != networks.end(); ++it, ++net_number) {
				net[net_number][feature_pattern(temp, *it)] += adjust;
			}
			temp.reflect_horizontal();
			temp.rotate();
			rotate += 1;
		}
		*/
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.reflect_horizontal();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
		temp.rotate();
		net[1][feature_pattern(temp, net1)] += adjust;
		net[2][feature_pattern(temp, net2)] += adjust;
		net[3][feature_pattern(temp, net3)] += adjust;
		net[4][feature_pattern(temp, net4)] += adjust;
			
	}
	void printNet(board& after) {
		board temp = after;
		int net_number = 1;
		std::vector<std::vector<int> >::iterator it;
		for(it = networks.begin(); it!= networks.end(); ++it, ++net_number){
			std::cout << "net: " << net_number << ", " << feature_pattern(temp, *it) << std::endl;
			std::cout<<net[net_number][feature_pattern(temp, *it)] << std::endl;
		}
	}

protected:
	virtual void init_weights(const std::string& info) {
		std::string res = info; // comma-separated sizes, e.g., "65536,65536"
		for (char& ch : res)
			if (!std::isdigit(ch)) ch = ' ';
		std::stringstream in(res);
		for (size_t size; in >> size; net.emplace_back(size));
		//net.emplace_back(65536);
		//net.emplace_back(65536);
		//net.emplace_back(65536);
		//net.emplace_back(65536);
	}
	virtual void load_weights(const std::string& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		uint32_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		net.resize(size);
		for (weight& w : net) in >> w;
		in.close();
	}
	virtual void save_weights(const std::string& path) {
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}

protected:
	std::vector<weight> net;
	std::vector<int>net1{0, 1, 2, 3, 4, 5};
	std::vector<int>net2{4, 5, 6, 7, 8, 9};
	std::vector<int>net3{5, 6, 7, 9, 10, 11};
	std::vector<int>net4{9, 10, 11, 13, 14, 15};
	//std::vector<int>net5{0, 1, 4, 5, 6, 7};
	//std::vector<int>net6{0, 1, 2, 3};
	//std::vector<int>net7{4, 5, 6, 7};
	//std::vector<int>net6{0, 1, 2, 5, 6, 7};
	//std::vector<int>net7{0, 4, 5, 8, 9, 10};
	//std::vector<int>net8{0, 1, 2, 4, 5, 9};
	//std::vector<int>net9{0, 1, 2, 4, 6, 8};
	//std::vector<int>net10{0, 1, 2, 5, 9, 10};
	//std::vector<int>net11{0, 1, 5, 6, 7, 10};
	//std::vector<int>net12{0, 1, 5, 8, 9, 13};
	std::vector<std::vector<int> >networks{net1, net2, net3, net4};
	float alpha;
};

/**
 * default random environment, i.e., placer
 * place the hint tile and decide a new hint tile
 */
class random_placer : public random_agent {
public:
	random_placer(const std::string& args = "") : random_agent("name=place role=placer " + args) {
		spaces[0] = { 12, 13, 14, 15 };
		spaces[1] = { 0, 4, 8, 12 };
		spaces[2] = { 0, 1, 2, 3};
		spaces[3] = { 3, 7, 11, 15 };
		spaces[4] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	}

	virtual action take_action(const board& after) {
		std::vector<int> space = spaces[after.last()];
		std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;

			int bag[3], num = 0;
			for (board::cell t = 1; t <= 3; t++)
				for (size_t i = 0; i < after.bag(t); i++)
					bag[num++] = t;
			std::shuffle(bag, bag + num, engine);

			board::cell tile = after.hint() ?: bag[--num];
			board::cell hint = bag[--num];

			return action::place(pos, tile, hint);
		}
		return action();
	}

private:
	std::vector<int> spaces[5];
};

/**
 * random player, i.e., slider
 * select a legal action randomly
 */
class random_slider : public weight_agent {
public:
	random_slider(const std::string& args = "") : weight_agent("name=slide role=slider " + args),
		opcode({ 0, 1, 2, 3 }) {}

	virtual action take_action(const board& before) {
		//std::shuffle(opcode.begin(), opcode.end(), engine);
		//for (int op : opcode) {
		//	board::reward reward = board(before).slide(op);
		//	if (reward != -1) return action::slide(op);
		//}
		//
		/*
		int max_tile = 0, board_sum = 0, space_count = 0, count_one = 0, count_two = 0;
		for(int i : before) {
		    if(i==0) ++space_count;
		    if(i==1) ++count_one;
		    if(i==2) ++count_two;
		    board_sum += i;
		    max_tile = std::max(max_tile, i);
		}

		int game_board[16] = {0, 0, 0, 0,
				      0, 0, 0, 0,
				      0, 0, 0, 0,
				      0, 0, 0, 0};
		int cnt = 0;
		for(int i : before) game_board[cnt] = i, ++cnt;

		int score = 0, flag = 0;
	
			
			
		if(game_board[0]==max_tile) {
			flag = 4;
			if(game_board[1]==max_tile/2){
				flag = 3;
				if(game_board[2] == max_tile/4) {
					flag = 2;
					if(game_board[3]==max_tile/8)
						flag = 1;
				}
			}
		}	
		flag = 4;

		int max_reward_action = -1, max_reward = -0x3f3f3f3f;
		int game_is_over = 0;
		for(int op : opcode) {
			if(op != 4){
			int score0 = 0;
			board::reward reward = board(before).slide(op);
			int is_move = reward;
			if(reward == -1) ++game_is_over;
			score0 += flag*reward;

			if(op == 2) {
				score0 -= flag*reward/4;
			}
			else if(op == 1) {
				if(flag != 4) score0 -= reward/2;
				else score0 += reward;
			}
			else score0 += reward/2;

			if(count_one >= 3) score0 -= count_one*reward;
			reward += score0;

			if(max_reward < reward && is_move != -1) max_reward_action = op, max_reward = reward;
	
			}
		}
		if(game_is_over < 4) return action::slide(max_reward_action);
		*/


		int best_action = -1;
		int best_reward = -1;
		float best_value = -0x3f3f3f3f;
		board best_after;

		

		for(int op : opcode) {
			board after = before;
			//board::reward reward = board(after).slide(op);
				
			
			board::reward reward = after.slide(op);

			
			if(reward == -1) continue;
			float value = estimate_value(after);
			
			if(reward + value > best_reward + best_value) {
				best_action = op;
				best_reward = reward;
				best_value = value;
				best_after = after;
			}
		}
		if(best_action != -1) {
			history.emplace_back(steps{best_reward, best_after});
			return action::slide(best_action);
		}

		//adjust_net_value(history[history.size()-1].after, 0);
		//for(int i = history.size()-2; i >= 0; --i) {
		//	adjust_net_value(history[i].after, history[i+1].reward + estimate_value(history[i+1].after));	
		//}

		
		
		//history.clear();
		return action();
	}
	virtual void open_episode(const std::string& flag = "") {
		history.clear();
	}
	virtual void close_episode(const std::string& flag = "") {
		adjust_net_value(history[history.size()-1].after, 0);
		 for(int i = history.size()-2; i >= 0; --i) {
			 adjust_net_value(history[i].after, history[i+1].reward + estimate_value(history[i+1].after));
		 }
	}

private:
	std::array<int, 4> opcode;
	struct steps {
		int reward;
		board after;
	};
	std::vector<steps> history;
};
