#include "uos.hold_by_emission.hpp"

namespace UOS {

    void uos_hold_by_emission::setparams(int64_t begin, int64_t end, float multiplier) {
           //print("SETTIME","\n");
           //print("BEGIN ", begin, "\n");
           //print("END ", end, "\n");

           //check self-authentication
           require_auth(_self);

           check(0 < begin, "must be non-zero values");
           check(begin < end, "begin must be less than end");
           check(multiplier > 0, "multiplier must be positive");

           //check if the params are already set
           check(!_limits.exists() || _multiplier.exists(), "the parameters are already set");

           //set values
           time_limits lim;
           lim.begin = begin;
           lim.end = end;
           _limits.set(lim,_self);

           emission_multiplier mult;
           mult.value = multiplier;
           _multiplier.set(mult,_self);
    }

    void uos_hold_by_emission::transfer(name from, name to, asset quantity, string memo) {
        //print("TRANSFER\n");
        //print("FROM ", name{from}, "\n");
        //print("TO ", name{to}, "\n");
        //print("QUANTITY ", quantity.to_string(), "\n");
        //print("AMOUNT ", quantity.amount, "\n");
        //print("SYMBOL CODE ", quantity.symbol.code().to_string(), "\n");
        //print("MEMO ", memo, "\n");

        if(from == _self) {
            //print("IGNORE OUTGOING TRANSFER\n");
            return;
        }

        check(quantity.symbol.code().to_string() == "UOS", "only UOS core token can be accepted");
        check(is_account(name{memo}), "must be an existing account name in memo");

        balance_table bals(_self,_self.value);
        auto acc_name = eosio::name(memo);
        auto itr = bals.find(acc_name.value);
        if(itr != bals.end()) {
            //print("FOUND!!!!!\n");
            bals.modify(itr,_self, [&] (balance_entry &item){
                item.acc_name=acc_name;
                item.deposit=itr->deposit + quantity.amount;
                item.withdrawal=itr->withdrawal;
            });
            //print("AND MODIFIED\n");
        }
        else {
            //print("NOT FOUND!!!!!\n");
            bals.emplace(_self,[&] (balance_entry &item){
                item.acc_name=acc_name;
                item.deposit=quantity.amount;
                item.withdrawal=0;
            });
            //print("AND ADDED\n");
        }
    }

    void uos_hold_by_emission::withdraw(name acc_name, asset emission, std::vector<checksum_pair> proof) {
        //print("WITHDRAW","\n");
        //print("ACC_NAME ", name{acc_name}, "\n");
        
        require_auth(acc_name);
        
        auto lim_begin = _limits.get().begin;
        //print("BEGIN ", lim_begin, "\n");
        auto lim_end = _limits.get().end;
        //print("END ", lim_end, "\n");
        auto mult = _multiplier.get().value;
        //print("MULTIPLIER ", mult, "\n");
        
        check(0 < lim_begin && lim_begin < lim_end, "limits are not set properly");

        check(check_proof(emission, proof), "proof failed the check");
        
        balance_table bals(_self,_self.value);
        auto itr = bals.find(acc_name.value);
        check(itr != bals.end(), "balance record not found");

        auto current_time = eosio::current_time_point().time_since_epoch()._count;
        //print("CURRENT TIME ", current_time, "\n");

        check(current_time > lim_begin, "withdrawal period not started yet");

        uint64_t limit_by_time = 0;
        if(current_time > lim_end) {
            //print("FULL DEPOSIT \n");
            limit_by_time = itr->deposit;
        } else {
            //print("SOME PART OF DEPOSIT \n");
            limit_by_time = (uint64_t)((float)itr->deposit
                                      * (float)(current_time - lim_begin)
                                      / (float)(lim_end - lim_begin));
        }

        uint64_t limit_by_emission = (uint64_t)((float)emission.amount * mult);

        uint64_t withdraw_limit = limit_by_time;
        if(limit_by_emission < withdraw_limit){
            withdraw_limit = limit_by_emission;
        }

        //print("DEPOSIT ", itr->deposit, "\n");
        //print("WITHDRAWAL ", itr->withdrawal, "\n");
        //print("WITHDRAW LIMIT ", withdraw_limit, "\n");

        check(itr->withdrawal < withdraw_limit, "nothing to withdraw");

        uint64_t current_withdrawal = withdraw_limit - itr->withdrawal;
        //print("CURRENT WITHDRAWAL ", current_withdrawal, "\n");
        
        //send current_withdrawal tokens to account
        asset ast(current_withdrawal, symbol("UOS",4));
        action(
            permission_level{ _self, name{"active"} },
            name{"eosio.token"}, name{"transfer"},
            std::make_tuple(_self, acc_name, ast, string("release from hold"))
        ).send();
        
        //increase withdrawal value
        bals.modify(itr,_self, [&] (balance_entry &item){
            item.acc_name=itr->acc_name;
            item.deposit=itr->deposit;
            item.withdrawal=itr->withdrawal + current_withdrawal;
        });        
    }

    bool uos_hold_by_emission::check_proof(asset emission, std::vector<checksum_pair> proof) {
        return true;
    }
    
    extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        uos_hold_by_emission _uos_hold_by_emission(name(receiver));
        if(code==receiver && action== name("setparams").value) {
            execute_action(name(receiver), name(code), &uos_hold_by_emission::setparams );
        }
        else if(code==receiver && action== name("withdraw").value) {
            execute_action(name(receiver), name(code), &uos_hold_by_emission::withdraw );
        }
        else if(code==name("eosio.token").value && action== name("transfer").value) {
        execute_action(name(receiver), name(code), &uos_hold_by_emission::transfer );
        }
    }
    }
}
