.. _manual_platform_sdk_in_app_purchases:

In-App Purchases
================

The Meta Platform SDK provides developers with an easy way to improve the monetization
of an application through the sale of In-App Purchases (IAP). APIs are provided to
easily handle the browsing, purchasing, and entitlement of an app's available prodcuts.

This page provides some examples to help developers utilize Platform SDK IAP
features in their projects. For more info on how to initially set up IAP in an app,
see the Meta documentation pages for:

- `Add-ons (DLC and IAP) <https://developers.meta.com/horizon/resources/add-ons>`_
- `Subscriptions <https://developers.meta.com/horizon/resources/subscriptions>`_

.. note::

    It's common for IAP to be associated with Downloadable Content (DLC). To learn
    more about the DLC features of the Platform SDK, see the page on :doc:`downloadable_content`.

Checking User Purchases
-----------------------

To know which products a user has ownership of, first obtain a list of user purchases
using :ref:`iap_get_viewer_purchases_async<class_metaplatformsdk_method_iap_get_viewer_purchases_async>`,
then examine the SKUs of the returned items.

.. note::

    In order to successfully retrieve user purchases for an app, the app must reside
    in the user's app library. This requires that a version of the app must be published
    on at least one release channel. See Meta's documentation on `Release Channels <https://developers.meta.com/horizon/resources/publish-release-channels/>`_
    for more information.

.. code-block:: gdscript

    var result := await MetaPlatformSDK.iap_get_viewer_purchases_async().completed
    if result.is_error():
        # An error occurred getting user purchases.
        return

    var purchase_array := result.get_purchase_array()
    for purchase in purchase_array:
        # Check purchase.sku here to determine what products a user owns.
        pass

Purchasing a Product
--------------------

To initiate a purchase for a user, simply launch the checkout flow with
:ref:`iap_launch_checkout_flow_async<class_metaplatformsdk_method_iap_launch_checkout_flow_async>`.

.. note::

    If checkout flow is launched for a product that a user already owns, checkout
    flow will automatically be aborted.

.. code-block:: gdscript

    var result := await MetaPlatformSDK.iap_launch_checkout_flow_async(PRODUCT_SKU).completed
    if result.is_error():
        # An error occurred during checkout flow.
        return

    # Product has been purchased successfully.
    var purchase := result.get_purchase()

Consumable Purchases
--------------------

Consumable addons may be purchased multiple times by users, but they must be consumed before
being purchased again. To consume one of these products, provide the addon's sku to
:ref:`iap_consume_purchase_async<class_metaplatformsdk_method_iap_consume_purchase_async>`.

.. code-block:: gdscript

    var result := await MetaPlatformSDK.iap_consume_purchase_async(CONSUMABLE_ADDON_SKU).completed
    if result.is_error():
        # Error consuming consumable addon.
        return

    # Consumable successfully consumed.

Once consumed, the addon's product will no longer be present in the returned array of purchases,
and purchasing it will once again be possible.
