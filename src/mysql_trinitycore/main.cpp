#include "Implementation/SakilaDatabase.h"

#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "Log.h"

#include "MySQLThreading.h"
#include "AsyncCallbackProcessor.h"
#include "QueryHolder.h"

#include <iostream>

int main(int argc, char *argv[])
{
    MySQL::Library_Init();

    DatabaseLoader loader("server.sakilaserver", DatabaseLoader::DATABASE_NONE);
    loader.AddDatabase(SakilaDatabase, "127.0.0.1;3306;root;System123@;sakila", 8, 2);

    if (!loader.Load())
    {
        TC_LOG_ERROR("", "SakilaDatabase connect error");
        return 1;
    }
    TC_LOG_INFO("", "SakilaDatabase connect success");

    // {
    //     // SakilaDatabase.DirectExecute("INSERT INTO actor (first_name, last_name) VALUES ('mark', '0voice');");
    //     auto result = SakilaDatabase.Query("SELECT actor_id, first_name, last_name, last_update FROM actor "
    //                                        "WHERE actor_id BETWEEN 201 AND 303;");
    //     if (!result)
    //     {
    //         TC_LOG_ERROR("", "Query returned no results");
    //     }
    //     do
    //     {
    //         Field *fields = result->Fetch(); /// 获取当前行的字段
    //         TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}", fields[0].GetUInt32(),
    //                     fields[1].GetString().c_str(), fields[2].GetString().c_str(), fields[3].GetString());
    //     }
    //     while (result->NextRow());
    // }

    {
        /// 事务处理只针对写操作， 拿不到操作的结果
        auto trans = SakilaDatabase.BeginTransaction();
        trans->Append("INSERT INTO actor (first_name, last_name) VALUES ('king', '0voice')");
        trans->Append("INSERT INTO actor (first_name, last_name) VALUES ('darren', '0voice')");
        trans->Append("INSERT INTO country (country) VALUES ('china')");
        trans->Append("INSERT INTO city (city, country_id) VALUES ('chang sha', '109')");


        AsyncCallbackProcessor<TransactionCallback> processor;
        processor.AddCallback(SakilaDatabase.AsyncCommitTransaction(trans))
                .AfterComplete(
                        [](bool success)
                        {
                            if (success)
                            {
                                TC_LOG_INFO("", "transaction commit success");
                            }
                            else
                            {
                                TC_LOG_INFO("", "transaction commit failed");
                            }
                        });

        while (true)
        {
            processor.ProcessReadyCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    {
        class ActorInfoHolder : public SQLQueryHolder<SakilaDatabaseConnection>
        {
        public:
            enum ACTOR_INFO_IDX : uint8
            {
                AI_1,
                AI_3,
                AI_5,
                AI_MAX
            };

        public:
            ActorInfoHolder()
            {
                SetSize(AI_MAX);
                auto stmt8 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
                stmt8->setUInt32(0, 9);
                SetPreparedQuery(AI_1, stmt8);

                auto stmt9 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
                stmt9->setUInt32(0, 10);
                SetPreparedQuery(AI_3, stmt9);

                auto stmt10 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
                stmt10->setUInt32(0, 11);
                SetPreparedQuery(AI_5, stmt10);
            }
        };

        AsyncCallbackProcessor<SQLQueryHolderCallback> processor;
        auto                                           holder = std::make_shared<ActorInfoHolder>();
        processor.AddCallback(SakilaDatabase.DelayQueryHolder(holder))
                .AfterComplete(
                        [](SQLQueryHolderBase const &hdr)
                        {
                            auto holder  = dynamic_cast<ActorInfoHolder const *>(&hdr);
                            auto result1 = holder->GetPreparedResult(ActorInfoHolder::AI_1);
                            auto result2 = holder->GetPreparedResult(ActorInfoHolder::AI_3);
                            auto result3 = holder->GetPreparedResult(ActorInfoHolder::AI_5);
                            if (result1)
                            {
                                Field *fields = result1->Fetch();
                                TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
                                            fields[0].GetUInt32(), fields[1].GetString().c_str(),
                                            fields[2].GetString().c_str(), Trinity::DateFormat(fields[3].GetDate()));
                            }
                            else
                            {
                                TC_LOG_ERROR("", "Query 1 returned no results");
                            }
                            if (result2)
                            {
                                Field *fields = result2->Fetch();
                                TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
                                            fields[0].GetUInt32(), fields[1].GetString().c_str(),
                                            fields[2].GetString().c_str(), Trinity::DateFormat(fields[3].GetDate()));
                            }
                            else
                            {
                                TC_LOG_ERROR("", "Query 2 returned no results");
                            }
                            if (result3)
                            {
                                Field *fields = result3->Fetch();
                                TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
                                            fields[0].GetUInt32(), fields[1].GetString().c_str(),
                                            fields[2].GetString().c_str(), Trinity::DateFormat(fields[3].GetDate()));
                            }
                            else
                            {
                                TC_LOG_ERROR("", "Query 3 returned no results");
                            }
                        });

        while (true)
        {
            processor.ProcessReadyCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // {
    //     auto *stmt = SakilaDatabase.GetPreparedStatement(SakilaDatabaseConnection::Statements::SAKILA_SEL_ACTOR_INFO);
    //     stmt->setUInt32(0, 1); /// 设置参数 actor_id = 1
    //     auto result = SakilaDatabase.Query(stmt);
    //     if (!result)
    //     {
    //         TC_LOG_ERROR("", "Query returned no results");
    //     }
    //     do
    //     {
    //         Field *fields = result->Fetch(); /// 获取当前行的字段
    //         TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}", fields[0].GetUInt32(),
    //                     fields[1].GetString().c_str(), fields[2].GetString().c_str(),
    //                     Trinity::DateFormat(fields[3].GetDate()));
    //     }
    //     while (result->NextRow());
    // }

    //////////////////////////////////////////////////////////////////
    {
        AsyncCallbackProcessor<QueryCallback> processor;

        // auto *stmt = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        // stmt->setUInt32(0, 2); /// 设置参数 actor_id = 2
        // processor.AddCallback(SakilaDatabase.AsyncQuery(stmt).WithPreparedCallback(
        //         [](PreparedQueryResult result)
        //         {
        //             Field *fields = result->Fetch(); /// 获取当前行的字段
        //             TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
        //                         fields[0].GetUInt32(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
        //                         Trinity::DateFormat(fields[3].GetDate()));
        //         }));
        //
        // auto *stmt2 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        // stmt2->setUInt32(0, 3); /// 设置参数 actor_id = 3
        // processor.AddCallback(SakilaDatabase.AsyncQuery(stmt2).WithPreparedCallback(
        //         [](PreparedQueryResult result)
        //         {
        //             Field *fields = result->Fetch(); /// 获取当前行的字段
        //             TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
        //                         fields[0].GetUInt32(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
        //                         Trinity::DateFormat(fields[3].GetDate()));
        //         }));
        //
        // auto *stmt3 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        // stmt3->setUInt32(0, 4); /// 设置参数 actor_id = 4
        // processor.AddCallback(SakilaDatabase.AsyncQuery(stmt3).WithPreparedCallback(
        //         [](PreparedQueryResult result)
        //         {
        //             Field *fields = result->Fetch(); /// 获取当前行的字段
        //             TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
        //                         fields[0].GetUInt32(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
        //                         Trinity::DateFormat(fields[3].GetDate()));
        //         }));

        //////////////////////////////////////////////////////////////////

        auto *stmt4 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        stmt4->setUInt32(0, 5);

        auto *stmt5 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        stmt5->setUInt32(0, 6);

        auto *stmt6 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        stmt6->setUInt32(0, 7);

        auto *stmt7 = SakilaDatabase.GetPreparedStatement(SAKILA_SEL_ACTOR_INFO_ASYNC);
        stmt7->setUInt32(0, 8);

        processor.AddCallback(
                SakilaDatabase.AsyncQuery(stmt4)
                        .WithChainingPreparedCallback(
                                [stmt5](QueryCallback &callback, PreparedQueryResult result)
                                {
                                    std::cout << "555555555555555555555555" << std::endl;
                                    Field *fields = result->Fetch();
                                    TC_LOG_INFO("", "actor_id: {}", fields[0].GetUInt16());

                                    callback.SetNextQuery(SakilaDatabase.AsyncQuery(stmt5));
                                })
                        .WithChainingPreparedCallback(
                                [&](QueryCallback &callback2, PreparedQueryResult result2)
                                {
                                    std::cout << "666666666666666666666666666666" << std::endl;
                                    Field *fields2 = result2->Fetch();
                                    TC_LOG_INFO("", "actor_id: {}", fields2[0].GetUInt16());

                                    processor.AddCallback(SakilaDatabase.AsyncQuery(stmt6).WithPreparedCallback(
                                            [](PreparedQueryResult result3)
                                            {
                                                std::cout << "77777777777777777777777777777" << std::endl;
                                                Field *fields3 = result3->Fetch();
                                                TC_LOG_INFO("", "actor_id: {}", fields3[0].GetUInt16());
                                            }));
                                }));

        while (true)
        {
            processor.ProcessReadyCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }


    // /// 线程1：普通查询，遍历多行结果
    // std::thread thrd1(
    //         []()
    //         {
    //             // auto result = SakilaDatabase.Query("SELECT actor_id, first_name, last_name, last_update FROM actor "
    //             //                                    "WHERE actor_id BETWEEN 100 AND 103;");
    //             auto result = SakilaDatabase.Query(
    //                     "SELECT actor_id, first_name, last_name, last_update FROM actor WHERE actor_id = 100;");
    //             if (!result)
    //             {
    //                 TC_LOG_ERROR("", "Query returned no results");
    //                 return;
    //             }
    //
    //             // 遍历所有结果行
    //             do
    //             {
    //                 Field *fields = result->Fetch(); // 获取当前行的字段
    //                 TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
    //                             fields[0].GetUInt16(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
    //                             fields[3].GetString());
    //             }
    //             while (result->NextRow());
    //         });
    //
    // /// 线程2：预处理语句查询 actor_id = 101
    // std::thread thrd2(
    //         []()
    //         {
    //             auto result = SakilaDatabase.Query(
    //                     "SELECT actor_id, first_name, last_name, last_update FROM actor WHERE actor_id = 101;");
    //             do
    //             {
    //                 Field *fields = result->Fetch();
    //                 TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
    //                             fields[0].GetUInt16(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
    //                             fields[3].GetString().c_str());
    //             }
    //             while (result->NextRow());
    //         });
    //
    // /// 线程3：预处理语句查询 actor_id = 103
    // std::thread thrd3(
    //         []()
    //         {
    //             auto *stmt = SakilaDatabase.GetPreparedStatement(
    //                     SakilaDatabaseConnection::Statements::SAKILA_SEL_ACTOR_INFO);
    //             stmt->setUInt32(0, 103); // 设置参数 actor_id = 103
    //             auto result = SakilaDatabase.Query(stmt);
    //
    //             do
    //             {
    //                 Field *fields = result->Fetch();
    //                 TC_LOG_INFO("", "actor_id: {}, first_name: {}, last_name: {}, last_update: {}",
    //                             fields[0].GetUInt32(), fields[1].GetString().c_str(), fields[2].GetString().c_str(),
    //                             Trinity::DateFormat(fields[3].GetDate()));
    //             }
    //             while (result->NextRow());
    //         });
    //
    // thrd1.join();
    // thrd2.join();
    // thrd3.join();

    SakilaDatabase.Close();
    MySQL::Library_End();
    return 0;
}
