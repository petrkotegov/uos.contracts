#pragma once

#include <eosiolib/eosio.hpp>
//#include <eosiolib/print.hpp>
//#include <eosiolib/crypto.h>
//#include <eosiolib/public_key.hpp>
//#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
//#include <eosiolib/symbol.hpp>
#include <eosiolib/singleton.hpp>
//#include <eosio.token/eosio.token.hpp>
//#include <string>
//#include <cstdint>
//#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.hold")]]uos_hold : public eosio::contract {
    public:
        uos_hold(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _limits(code,receiver.value){

            if(!_limits.exists()){
                print("THERE IS NO LIMIT!!!!!\n");
                print("RECEIVER ", name{receiver}, "\n");
                print("CODE ", name{code}, "\n");
                print("_SELF ", name{_self}, "\n");
                    // time_limits temp;
                    // temp.begin = 0;
                    // temp.end = 0;
                    // _limits.set(temp,_self);
                }
        }

        [[eosio::action]]
        void settime(int64_t begin, int64_t end);

        [[eosio::action]]
        void transfer(name from, name to, asset quantity, string memo);

        [[eosio::action]]
        void deposit(name acc_name, eosio::asset amount);

        [[eosio::action]]
        void withdraw(name acc_name);

    private:

        struct  [[eosio::table("limits")]]
        time_limits{
            uint64_t begin;
            uint64_t end;
        };


        // struct  [[eosio::table]]
        // balance_entry {
        //     name acc_name;
        //     uint64_t deposit;
        //     uint64_t withdrawal;

        //     uint64_t primary_key() const { return acc_name.value; }

        //     EOSLIB_SERIALIZE(balance_entry, (acc_name)(deposit)(withdrawal))
        // };

        typedef eosio::singleton <"limits"_n, time_limits> time_limits_singleton;

        // typedef multi_index <"receipt"_n, rec_entry
        //         , indexed_by<"external.id"_n, const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_external_id>>
        //         , indexed_by<"acc.name"_n, const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_acc_name>>
        // > receipt_table;

        time_limits_singleton _limits;

    };
}