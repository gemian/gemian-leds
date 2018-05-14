/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "DBusClient.h"

#include "../src/core/DBusMessageHandle.h"
#include "../src/core/ScopedGError.h"

#include <stdexcept>

DBusAsyncReply::DBusAsyncReply(DBusAsyncReply&& other)
    : promise{std::move(other.promise)}
{
}

DBusAsyncReply::~DBusAsyncReply()
{
    throw_on_error_reply(get());
}

DBusMessageHandle DBusAsyncReply::get()
{
    if (pending)
    {
        auto future = promise.get_future();
        pending = false;
        return DBusMessageHandle{future.get()};
    }
    else
    {
        return DBusMessageHandle{nullptr};
    }
}

void DBusAsyncReply::set_pending()
{
    pending = true;
}

void DBusAsyncReply::static_set_async_result(
    GDBusConnection* connection, GAsyncResult* result, DBusAsyncReply* reply)
{
    auto const msg = g_dbus_connection_send_message_with_reply_finish(
        connection, result, nullptr);
    reply->set_async_result(msg);
}

void DBusAsyncReply::throw_on_error_reply(GDBusMessage* reply)
{
    if (reply && g_dbus_message_get_error_name(reply) != nullptr)
    {
        ScopedGError error;
        g_dbus_message_to_gerror(reply, error);

        throw std::runtime_error("Got an error reply: " + error.message_str());
    }
}

void DBusAsyncReply::throw_on_invalid_reply(GDBusMessage* reply)
{
    if (!reply)
        throw std::runtime_error("Async reply is invalid");
}

void DBusAsyncReply::set_async_result(GDBusMessage* message)
{
    promise.set_value(message);
}

void DBusAsyncReplyVoid::get()
{
    auto reply = DBusAsyncReply::get();
    throw_on_invalid_reply(reply);
    throw_on_error_reply(reply);
}

int DBusAsyncReplyInt::get()
{
    auto reply = DBusAsyncReply::get();
    throw_on_invalid_reply(reply);
    throw_on_error_reply(reply);

    auto body = g_dbus_message_get_body(reply);
    auto child = g_variant_get_child_value(body, 0);
    auto val = g_variant_get_int32(child);
    g_variant_unref(child);

    return val;
}

bool DBusAsyncReplyBool::get()
{
    auto reply = DBusAsyncReply::get();
    throw_on_invalid_reply(reply);
    throw_on_error_reply(reply);

    auto body = g_dbus_message_get_body(reply);
    auto child = g_variant_get_child_value(body, 0);
    auto val = g_variant_get_boolean(child);
    g_variant_unref(child);

    return val == TRUE;
}

std::string DBusAsyncReplyString::get()
{
    auto reply = DBusAsyncReply::get();
    throw_on_invalid_reply(reply);
    throw_on_error_reply(reply);

    auto body = g_dbus_message_get_body(reply);
    auto child = g_variant_get_child_value(body, 0);
    auto val = g_variant_get_string(child, nullptr);
    g_variant_unref(child);

    return std::string{val};
}

DBusClient::DBusClient(
    std::string const& bus_address,
    std::string const& destination,
    std::string const& path)
    : connection{bus_address.c_str()},
      event_loop{destination},
      destination{destination},
      path{path}
{
}

void DBusClient::DBusClient::disconnect()
{
    if (!g_dbus_connection_is_closed(connection))
        g_dbus_connection_close_sync(connection, nullptr, nullptr);
}

std::string DBusClient::DBusClient::unique_name()
{
    auto const str = g_dbus_connection_get_unique_name(connection);
    return str ? str : ":invalid";
}

void DBusClient::emit_signal(
    char const* interface, char const* name, GVariant* args)
{
    g_dbus_connection_emit_signal(
        connection,
        nullptr,
        path.c_str(),
        interface,
        name,
        args,
        nullptr);
}

void DBusClient::emit_signal_full(
    char const* path, char const* interface, char const* name, GVariant* args)
{
    g_dbus_connection_emit_signal(
        connection,
        nullptr,
        path,
        interface,
        name,
        args,
        nullptr);
}

void DBusClient::invoke_async(
    DBusAsyncReply* reply, char const* interface, char const* method, GVariant* args)
{
    static int const timeout_ms = 5000;
    reply->set_pending();

    event_loop.enqueue(
        [this, reply, interface, method, args]
        {
            DBusMessageHandle msg{
                g_dbus_message_new_method_call(
                    destination.c_str(),
                    path.c_str(),
                    interface,
                    method),
                args};


            g_dbus_connection_send_message_with_reply(
                connection,
                msg,
                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                timeout_ms,
                nullptr,
                nullptr,
                reinterpret_cast<GAsyncReadyCallback>(&DBusAsyncReply::static_set_async_result),
                reply);
        });
}
