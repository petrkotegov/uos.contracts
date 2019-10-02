#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.hold_by_emission")]]uos_hold_by_emission : public eosio::contract {
    public:
        typedef std::vector<checksum256> checksum_pair;

        uos_hold_by_emission(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _limits(code,receiver.value), _multiplier(code,receiver.value){
        }

        [[eosio::action]]
        void setparams(int64_t begin, int64_t end, float multiplier);

        [[eosio::action]]
        void transfer(name from, name to, asset quantity, string memo);

        [[eosio::action]]
        void withdraw(name acc_name, asset emission, std::vector<checksum_pair> proof);

    private:

        struct  [[eosio::table("params")]]
        parameters{
            uint64_t time_begin;
            uint64_t time_end;

            float multiplier;

            name emission_contract;
        };

        struct  [[eosio::table]]
        balance_entry {
            name acc_name;
            uint64_t deposit;
            uint64_t withdrawal;

            uint64_t primary_key() const { return acc_name.value; }

            EOSLIB_SERIALIZE(balance_entry, (acc_name)(deposit)(withdrawal))
        };

        typedef eosio::singleton <"params"_n, parameters> parameters_singleton;

        typedef multi_index <"balance"_n, balance_entry> balance_table;

        parameters _params;
    };
}